/*
 * Copyright (c) 2020-2025, Rauli Laine
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stack>

#include <peelo/unicode/encoding/utf8.hpp>

#include "linenoise.hpp"

#include "snek/cli/utils.hpp"
#include "snek/interpreter/runtime.hpp"

#if !defined(BUFSIZ)
#  define BUFSIZ 1024
#endif

namespace snek::cli
{
  using interpreter::Error;
  using interpreter::Runtime;
  using interpreter::Scope;

  static void
  CountOpenBraces(const std::string& input, std::stack<char>& open_braces)
  {
    const auto length = input.length();

    for (std::size_t i = 0; i < length; ++i)
    {
      const auto c = input[i];

      switch (c)
      {
        case '#':
          return;

        case '(':
          open_braces.push(')');
          break;

        case '[':
          open_braces.push(']');
          break;

        case '{':
          open_braces.push('}');
          break;

        case ')':
        case ']':
        case '}':
          if (!open_braces.empty() && open_braces.top() == c)
          {
            open_braces.pop();
          }
          break;

        case '\'':
        case '"':
          while (++i < length)
          {
            if (input[i] == c)
            {
              break;
            }
            else if (input[i] == '\\' && i + 1 < length)
            {
              ++i;
            }
          }
          break;
      }
    }
  }

  static inline bool
  IsBlank(const std::string& input)
  {
    for (const auto& c : input)
    {
      if (!std::isspace(c))
      {
        return false;
      }
    }

    return true;
  }

  void
  RunRepl(Runtime& runtime, const Scope::ptr& scope)
  {
    using peelo::unicode::encoding::utf8::encode;

    char prompt[BUFSIZ];
    std::stack<char> open_braces;
    int line = 1;
    std::string source;

    for (;;)
    {
      std::string input;
      bool quit;

      std::snprintf(
        prompt,
        sizeof(prompt),
        "snek:%d%c ",
        line++,
        open_braces.empty() ? '>' : '*'
      );
      input = linenoise::Readline(prompt, quit);
      if (quit)
      {
        break;
      }
      if (IsBlank(input))
      {
        continue;
      }
      linenoise::AddHistory(input.c_str());
      source.append(input).append(1, '\n');
      CountOpenBraces(input, open_braces);
      if (!open_braces.empty())
      {
        continue;
      }
      try
      {
        const auto value = runtime.RunScript(scope, source, U"<repl>", line);

        if (!interpreter::value::IsNull(value))
        {
          std::cout << encode(interpreter::value::ToSource(value)) << std::endl;
        }
      }
      catch (const Error& e)
      {
        utils::PrintStackTrace(std::cout, e);
      }
      source.clear();
    }
  }
}
