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
#include "snek/interpreter/runtime.hpp"

namespace snek::interpreter::prototype
{
  /**
   * Object#toString(this) => String
   *
   * Creates string representation of the object.
   */
  static value::ptr
  ToString(Runtime&, const std::vector<value::ptr>& arguments)
  {
    if (value::IsString(arguments[0]))
    {
      return arguments[0];
    }

    return value::String::Make(value::ToString(arguments[0]));
  }

  /**
   * Object#==(this, other) => Boolean
   *
   * Tests whether two objects are equal with each other.
   */
  static value::ptr
  Equals(Runtime&, const std::vector<value::ptr>& arguments)
  {
    return std::make_shared<value::Boolean>(value::Equals(
      arguments[0],
      arguments[1]
    ));
  }

  /**
   * Object#!=(this, other) => Boolean
   *
   * Negates return value of `==`. Can be used to test whether two objects are
   * not equal with each other.
   */
  static value::ptr
  NotEquals(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    return std::make_shared<value::Boolean>(!value::ToBoolean(
      value::CallMethod(
        runtime,
        arguments[0],
        U"==",
        { arguments[1] }
      )
    ));
  }

  void
  MakeObject(const Runtime* runtime, value::Record::container_type& fields)
  {
    fields[U"toString"] = value::Function::MakeNative(
      { { U"this" } },
      runtime->string_type(),
      ToString
    );

    fields[U"=="] = value::Function::MakeNative(
      { { U"this" }, { U"other" } },
      runtime->boolean_type(),
      Equals
    );
    fields[U"!="] = value::Function::MakeNative(
      { { U"this" }, { U"other" } },
      runtime->boolean_type(),
      NotEquals
    );
  }
}
