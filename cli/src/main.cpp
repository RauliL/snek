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
#include <cstring>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#if !defined(_WIN32)
#  include <unistd.h>
#endif

#include <peelo/unicode/encoding/utf8.hpp>

#include "snek/cli/utils.hpp"
#include "snek/interpreter/runtime.hpp"

using snek::interpreter::Error;
using snek::interpreter::Runtime;
using snek::interpreter::Scope;

namespace snek::cli
{
  void RunRepl(Runtime& runtime, const Scope::ptr& scope);
}

static std::optional<std::string> script;
static std::vector<std::string> inline_scripts;

static void
PrintUsage(std::ostream& output, const char* executable_name)
{
  output << std::endl
         << "Usage: "
         << executable_name
         << " [switches] [programfile]"
         << std::endl
         << "  -e program        One line of program. (Omit programfile.)"
         << std::endl
         << "  --version         Print the version."
         << std::endl
         << "  --help            Display this message."
         << std::endl
         << std::endl;
}

static void
ParseArgs(int argc, char** argv)
{
  int offset = 1;

  while (offset < argc)
  {
    auto arg = argv[offset++];

    if (!*arg)
    {
      continue;
    }
    else if (*arg != '-')
    {
      script = arg;
      break;
    }
    else if (!arg[1])
    {
      break;
    }
    else if (arg[1] == '-')
    {
      if (!std::strcmp(arg, "--help"))
      {
        PrintUsage(std::cout, argv[0]);
        std::exit(EXIT_SUCCESS);
      }
      else if (!std::strcmp(arg, "--version"))
      {
        // TODO: Output version.
        std::exit(EXIT_SUCCESS);
      } else {
        std::cerr << "Unrecognized switch: " << arg << std::endl;
        PrintUsage(std::cerr, argv[0]);
        std::exit(EXIT_FAILURE);
      }
    }

    for (int i = 1; arg[i]; ++i)
    {
      switch (arg[i])
      {
        case 'e':
          if (offset < argc)
          {
            inline_scripts.push_back(argv[offset++]);
          } else {
            std::cerr << "Argument expected for the -e option." << std::endl;
            PrintUsage(std::cerr, argv[0]);
            std::exit(EXIT_FAILURE);
          }
          break;

        case 'h':
          PrintUsage(std::cout, argv[0]);
          std::exit(EXIT_SUCCESS);
          break;

        default:
          std::cerr << "Unrecognized switch: " << arg[i] << std::endl;
          std::exit(EXIT_FAILURE);
          break;
      }
    }
  }

  if (offset < argc)
  {
    std::cerr << "Too many arguments given." << std::endl;
    PrintUsage(std::cerr, argv[0]);
    std::exit(EXIT_FAILURE);
  }
}

static void
RunScript(
  Runtime& runtime,
  const Scope::ptr& scope,
  const std::u32string& filename,
  const std::string& source
)
{
  try
  {
    runtime.RunScript(scope, source, filename);
  }
  catch (const Error& e)
  {
    snek::cli::utils::PrintStackTrace(std::cerr, e);
    std::exit(EXIT_FAILURE);
  }
}

static inline std::string
ReadStream(std::istream& stream)
{
  return std::string(
    std::istreambuf_iterator<char>(stream),
    std::istreambuf_iterator<char>()
  );
}

static void
RunFile(
  Runtime& runtime,
  const Scope::ptr& scope,
  const std::string& filename
)
{
  using peelo::unicode::encoding::utf8::decode;

  std::ifstream ifs(filename);
  std::string source;

  if (!ifs.good())
  {
    std::cerr << "Unable to open `"
              << filename
              << "' for reading."
              << std::endl;
    std::exit(EXIT_FAILURE);
  }
  source.append(ReadStream(ifs));
  ifs.close();
  RunScript(runtime, scope, decode(filename), source);
}

static inline bool
IsInteractiveTerminal()
{
#if defined(_WIN32)
  return true;
#else
  return isatty(fileno(stdin));
#endif
}

int
main(int argc, char** argv)
{
  Runtime runtime;
  const auto scope = std::make_shared<Scope>(runtime.root_scope());

  ParseArgs(argc, argv);

  // Define the magic variable used to detect whether an module is being
  // imported or not.
  scope->DeclareVariable(
    std::nullopt,
    U"__name__",
    snek::interpreter::value::String::Make(U"__main__")
  );

  if (!inline_scripts.empty())
  {
    for (const auto& inline_script : inline_scripts)
    {
      RunScript(runtime, scope, U"<arg>", inline_script);
    }
  }
  else if (script)
  {
    RunFile(runtime, scope, *script);
  }
  else if (IsInteractiveTerminal())
  {
    snek::cli::RunRepl(runtime, scope);
  } else {
    RunScript(runtime, scope, U"<stdin>", ReadStream(std::cin));
  }

  return EXIT_SUCCESS;
}
