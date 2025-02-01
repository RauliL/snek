/*
 * Copyright (c) 2025, Rauli Laine
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
#include "snek/interpreter/runtime.hpp"

namespace snek::interpreter::prototype
{
  inline const std::u32string&
  AsString(const value::ptr& value)
  {
    return static_cast<const value::String*>(value.get())->value();
  }

  /**
   * String#length(this: String) => Int
   *
   * Returns length of the string.
   */
  static value::ptr
  Length(
    Runtime&,
    const std::vector<value::ptr>& arguments
  )
  {
    return std::make_shared<value::Int>(AsString(arguments[0]).length());
  }

  /**
   * String#+(this: String, other: String) => String
   *
   * Concatenates two strings with each other.
   */
  static value::ptr
  Concatenate(
    Runtime&,
    const std::vector<value::ptr>& arguments
  )
  {
    return std::make_shared<value::String>(
      AsString(arguments[0]) + AsString(arguments[1])
    );
  }

  void
  MakeString(const Runtime* runtime, value::Record::container_type& fields)
  {
    fields[U"length"] = value::Function::MakeNative(
      {
        Parameter(U"this", runtime->string_type())
      },
      runtime->int_type(),
      Length
    );
    fields[U"+"] = value::Function::MakeNative(
      {
        Parameter(U"this", runtime->string_type()),
        Parameter(U"other", runtime->string_type())
      },
      runtime->string_type(),
      Concatenate
    );
  }
}
