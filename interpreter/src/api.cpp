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
#include <cstdlib>
#include <iostream>

#include <peelo/unicode/encoding/utf8.hpp>

#include "snek/interpreter/runtime.hpp"

namespace snek::interpreter::api
{
  /**
   * eval(source: String) => any
   *
   * Evaluates given string as Snek script and returns result.
   */
  static value::ptr
  Eval(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    return runtime.RunScript(
      std::make_shared<Scope>(runtime.root_scope()),
      static_cast<const value::String*>(arguments[0].get())->ToString()
    );
  }

  /**
   * exit(code: Int = 0) => null
   *
   * Exits the program with the given exit code.
   */
   static value::ptr
   Exit(Runtime&, const std::vector<value::ptr>& arguments)
   {
     std::exit(static_cast<const value::Int*>(arguments[0].get())->value);

     return nullptr;
   }

   /**
    * panic(message: String) => null
    *
    * Prints the given message to standard error stream and exits the program
    * with erroneous exit code.
    */
  static value::ptr
  Panic(Runtime&, const std::vector<value::ptr>& arguments)
  {
    using peelo::unicode::encoding::utf8::encode;

    std::cerr << encode(static_cast<const value::String*>(
      arguments[0].get()
    )->ToString()) << std::endl;
    std::exit(EXIT_FAILURE);

    return nullptr;
  }

  /**
   * print(...objects: any[]) => null
   *
   * Outputs string representation of given objects into standard output
   * stream, separated from each other with a whitespace character.
   */
  static value::ptr
  Print(Runtime&, const std::vector<value::ptr>& arguments)
  {
    using peelo::unicode::encoding::utf8::encode;

    const auto list = static_cast<const value::List*>(arguments[0].get());
    const auto size = list->GetSize();
    std::u32string string;

    for (std::size_t i = 0; i < size; ++i)
    {
      if (i > 0)
      {
        string.append(1, U' ');
      }
      string.append(value::ToString(list->At(i)));
    }

    std::cout << encode(string) << std::endl;

    return nullptr;
  }

  void
  AddGlobalVariables(
    const Runtime* runtime,
    Scope::variable_container_type& variables
  )
  {
    variables[U"eval"] =
    {
      value::Function::MakeNative(
        { { U"source", runtime->string_type() } },
        runtime->any_type(),
        Eval
      ),
      true
    };
    variables[U"exit"] =
    {
      value::Function::MakeNative(
        {
          {
            U"code",
            runtime->int_type(),
            std::make_shared<parser::expression::Int>(
              std::nullopt,
              EXIT_SUCCESS
            )
          },
        },
        runtime->void_type(),
        Exit
      ),
      true
    };
    variables[U"panic"] =
    {
      value::Function::MakeNative(
        { { U"message", runtime->string_type() } },
        runtime->void_type(),
        Panic
      ),
      true
    };
    variables[U"print"] =
    {
      value::Function::MakeNative(
        { { U"objects", runtime->list_type(), nullptr, true } },
        runtime->void_type(),
        Print
      ),
      true
    };
  }
}
