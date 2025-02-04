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
#include "snek/error.hpp"
#include "snek/interpreter/runtime.hpp"
#include "snek/parser/utils.hpp"

namespace snek::interpreter::prototype
{
  template<class T>
  static inline const T*
  As(const value::ptr& value)
  {
    return static_cast<const T*>(value.get());
  }

  /**
   * Record#[](this: Record, name: String) => any
   *
   * Returns value of field with given name contained in the record.
   */
  static value::ptr
  At(
    Runtime&,
    const std::vector<value::ptr>& arguments
  )
  {
    const auto& key = As<value::String>(arguments[1])->value();
    const auto result = As<value::Record>(arguments[0])->GetOwnProperty(key);

    if (result)
    {
      return *result;
    }

    throw Error{
      std::nullopt,
      value::ToString(value::KindOf(arguments[0])) +
      U" has no property `" +
      parser::utils::ToJsonString(key) +
      U"'."
    };
  }

  void
  MakeRecord(const Runtime* runtime, value::Record::container_type& fields)
  {
    fields[U"[]"] = value::Function::MakeNative(
      {
        { U"this", runtime->record_type() },
        { U"name", runtime->string_type() },
      },
      runtime->any_type(),
      At
    );
  }
}
