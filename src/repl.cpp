/*
 * Copyright (c) 2020, Rauli Laine
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

#include <peelo/prompt.hpp>

#include <snek/interpreter.hpp>
#include <snek/scope.hpp>

namespace snek::repl
{
  static inline bool
  is_blank(const std::string& input)
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

  static void
  count_open_braces(
    const std::string& input,
    std::stack<char32_t>& open_braces
  )
  {
    const auto length = input.length();

    // TODO: Add support for Pythonic blocks.
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
          if (!open_braces.empty()
              && open_braces.top() == static_cast<char32_t>(c))
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

  void
  loop(const Interpreter::permission_container_type& permissions)
  {
    using peelo::unicode::encoding::utf8::decode;
    using peelo::unicode::encoding::utf8::encode;
    Interpreter interpreter(permissions);
    Scope scope;
    peelo::prompt prompt;
    int line = 0;
    std::u32string source;
    std::stack<char32_t> open_braces;
    char prompt_string[BUFSIZ];

    prompt.set_completion_callback(
      [&scope](
        const std::string& buffer,
        std::vector<std::string>& completions
      )
      {
        std::u32string decoded_buffer;

        if (buffer.empty())
        {
          return;
        }
        decoded_buffer = decode(buffer);
        for (const auto& variable : scope.variables())
        {
          if (variable.first.rfind(decoded_buffer, 0) == 0)
          {
            completions.push_back(encode(variable.first));
          }
        }
        for (const auto& type : scope.types())
        {
          if (type.first.rfind(decoded_buffer, 0) == 0)
          {
            completions.push_back(encode(type.first));
          }
        }
      }
    );

    for (;;)
    {
      std::snprintf(
        prompt_string,
        BUFSIZ,
        "snek:%d%c ",
        ++line,
        open_braces.empty() ? '>' : '*'
      );

      if (const auto line = prompt.input(prompt_string))
      {
        // Skip empty lines.
        if (is_blank(*line))
        {
          continue;
        }
        prompt.add_to_history(*line);
        source.append(decode(*line));
        source.append(1, '\n');
        count_open_braces(*line, open_braces);
      } else {
        break;
      }

      if (!open_braces.empty())
      {
        continue;
      }

      const auto result = interpreter.eval(source, scope, U"<repl>", line);

      if (result)
      {
        if (result.value()->kind() != value::Kind::Null)
        {
          std::cout << encode(result.value()->to_string()) << std::endl;
        }
      } else {
        std::cout << result.error() << std::endl;
      }

      source.clear();
    }
  }
}
