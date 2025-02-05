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
#include <peelo/unicode/ctype/tolower.hpp>
#include <peelo/unicode/ctype/toupper.hpp>

#include "snek/error.hpp"
#include "snek/interpreter/runtime.hpp"

namespace snek::interpreter::prototype
{
  static inline const value::String*
  AsString(const value::ptr& value)
  {
    return static_cast<const value::String*>(value.get());
  }

  static value::String::size_type
  AsIndex(const value::String* string, const value::ptr& index_value)
  {
    const auto length = string->GetLength();
    auto index = static_cast<const value::Int*>(index_value.get())->value();

    if (index < 0)
    {
      index += length;
    }
    if (!length || length < 0 || index >= static_cast<std::int64_t>(length))
    {
      throw Error{ std::nullopt, U"String index out of bounds." };
    }

    return static_cast<value::String::size_type>(index);
  }

  /**
   * String#codePointAt(this: String, index: Int) => Int
   *
   * Returns Unicode code point from given index.
   */
  static value::ptr
  CodePointAt(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    const auto string = AsString(arguments[0]);
    const auto index = AsIndex(string, arguments[1]);

    return runtime.MakeInt(static_cast<std::int64_t>(string->At(index)));
  }

  /**
   * String#length(this: String) => Int
   *
   * Returns length of the string.
   */
  static value::ptr
  Length(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    return runtime.MakeInt(AsString(arguments[0])->GetLength());
  }

  /**
   * String#reverse(this: String) => String
   *
   * Returns reversed copy of the string.
   */
  static value::ptr
  Reverse(Runtime&, const std::vector<value::ptr>& arguments)
  {
    return value::String::Reverse(
      std::static_pointer_cast<value::String>(arguments[0])
    );
  }

  /**
   * String#toLower(this: String) => String
   *
   * Converts string into lower case.
   */
  static value::ptr
  ToLower(Runtime&, const std::vector<value::ptr>& arguments)
  {
    using peelo::unicode::ctype::tolower;

    const auto s = AsString(arguments[0]);
    const auto length = s->GetLength();
    std::u32string result;

    result.reserve(length);
    for (std::size_t i = 0; i < length; ++i)
    {
      result.append(1, tolower(s->At(i)));
    }

    return value::String::Make(result);
  }

  /**
   * String#toUpper(this: String) => String
   *
   * Converts string into upper case.
   */
  static value::ptr
  ToUpper(Runtime&, const std::vector<value::ptr>& arguments)
  {
    using peelo::unicode::ctype::toupper;

    const auto s = AsString(arguments[0]);
    const auto length = s->GetLength();
    std::u32string result;

    result.reserve(length);
    for (std::size_t i = 0; i < length; ++i)
    {
      result.append(1, toupper(s->At(i)));
    }

    return value::String::Make(result);
  }

  /**
   * String#+(this: String, other: String) => String
   *
   * Concatenates two strings with each other.
   */
  static value::ptr
  Concatenate(Runtime&, const std::vector<value::ptr>& arguments)
  {
    return value::String::Concat(
      std::static_pointer_cast<value::String>(arguments[0]),
      std::static_pointer_cast<value::String>(arguments[1])
    );
  }

  /**
   * String#[](this: String, index: Int) => String
   *
   * Returns character from given index.
   */
  static value::ptr
  At(Runtime&, const std::vector<value::ptr>& arguments)
  {
    const auto string = AsString(arguments[0]);
    const auto index = AsIndex(string, arguments[1]);

    return value::String::Make(std::u32string(1, string->At(index)));
  }

  void
  MakeString(const Runtime* runtime, value::Record::container_type& fields)
  {
    fields[U"codePointAt"] = value::Function::MakeNative(
      {
        { U"this", runtime->string_type() },
        { U"index", runtime->int_type() },
      },
      runtime->int_type(),
      CodePointAt
    );
    fields[U"length"] = value::Function::MakeNative(
      { { U"this", runtime->string_type() } },
      runtime->int_type(),
      Length
    );
    fields[U"reverse"] = value::Function::MakeNative(
      { { U"this", runtime->string_type() } },
      runtime->string_type(),
      Reverse
    );
    fields[U"toLower"] = value::Function::MakeNative(
      { { U"this", runtime->string_type() } },
      runtime->string_type(),
      ToLower
    );
    fields[U"toUpper"] = value::Function::MakeNative(
      { { U"this", runtime->string_type() } },
      runtime->string_type(),
      ToUpper
    );

    fields[U"+"] = value::Function::MakeNative(
      {
        { U"this", runtime->string_type() },
        { U"other", runtime->string_type() }
      },
      runtime->string_type(),
      Concatenate
    );

    fields[U"[]"] = value::Function::MakeNative(
      {
        { U"this", runtime->string_type() },
        { U"index", runtime->int_type() },
      },
      runtime->string_type(),
      At
    );
  }
}
