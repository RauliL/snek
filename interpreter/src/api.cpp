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
#include <iostream>

#include <peelo/unicode/encoding/utf8.hpp>

#include "snek/interpreter/runtime.hpp"

namespace snek::interpreter::api
{
  /**
   * print(object: any) => null
   *
   * Outputs string representation of given object into standard output stream.
   */
  static value::ptr
  Print(
    Runtime&,
    const std::vector<value::ptr>& arguments
  )
  {
    using peelo::unicode::encoding::utf8::encode;

    std::cout << encode(value::ToString(arguments[0])) << std::endl;

    return nullptr;
  }

  void
  AddGlobalVariables(
    const Runtime* runtime,
    Scope::variable_container_type& variables
  )
  {
    variables[U"print"] =
    {
      value::Function::MakeNative(
        { Parameter(U"object", runtime->any_type()) },
        runtime->void_type(),
        Print
      ),
      true
    };
  }
}
