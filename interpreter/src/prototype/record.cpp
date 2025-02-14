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
#include "snek/interpreter/error.hpp"
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
   * Record#entries(this: Record) => [String, any][]
   *
   * Returns all non-inherited fields that the record has.
   */
  static value::ptr
  Entries(Runtime&, const std::vector<value::ptr>& arguments)
  {
    const auto record = As<value::Record>(arguments[0]);
    std::vector<value::ptr> result;

    for (const auto& field : record->fields())
    {
      result.push_back(value::List::Make({
        value::String::Make(field.first),
        field.second,
      }));
    }

    return value::List::Make(result);
  }

  /**
   * Record#keys(this: Record) => String[]
   *
   * Returns all non-inherited field names that the record has.
   */
  static value::ptr
  Keys(Runtime&, const std::vector<value::ptr>& arguments)
  {
    const auto record = As<value::Record>(arguments[0]);
    std::vector<value::ptr> result;

    for (const auto& field : record->fields())
    {
      result.push_back(value::String::Make(field.first));
    }

    return value::List::Make(result);
  }

  /**
   * Record#values(this: Record) => List
   *
   * Returns all non-inherited field values that the record has.
   */
  static value::ptr
  Values(Runtime&, const std::vector<value::ptr>& arguments)
  {
    const auto record = As<value::Record>(arguments[0]);
    std::vector<value::ptr> result;

    for (const auto& field : record->fields())
    {
      result.push_back(field.second);
    }

    return value::List::Make(result);
  }

  /**
   * Record#+(this: Record, other: Record) => Record
   *
   * Combines fields of two records into a new record.
   */
  static value::ptr
  Concat(Runtime&, const std::vector<value::ptr>& arguments)
  {
    const auto r1 = As<value::Record>(arguments[0]);
    const auto r2 = As<value::Record>(arguments[1]);
    value::Record::container_type result(r1->fields());

    for (const auto& field : r2->fields())
    {
      result[field.first] = field.second;
    }

    return std::make_shared<value::Record>(result);
  }

  /**
   * Record#-(this: Record, field: String) => Record
   *
   * Removes field from the record.
   */
  static value::ptr
  Remove(Runtime&, const std::vector<value::ptr>& arguments)
  {
    auto fields = As<value::Record>(arguments[0])->fields();
    const auto key = As<value::String>(arguments[1])->ToString();
    const auto it = fields.find(key);

    if (it != std::end(fields))
    {
      fields.erase(it);
    }

    return std::make_shared<value::Record>(fields);
  }

  /**
   * Record#[](this: Record, name: String) => any
   *
   * Returns value of field with given name contained in the record.
   */
  static value::ptr
  At(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    const auto key = As<value::String>(arguments[1])->ToString();
    const auto result = As<value::Record>(arguments[0])->GetOwnProperty(key);

    if (result)
    {
      return *result;
    }

    throw runtime.MakeError(
      value::ToString(value::KindOf(arguments[0])) +
      U" has no property `" +
      parser::utils::ToJsonString(key) +
      U"'."
    );
  }

  void
  MakeRecord(const Runtime* runtime, value::Record::container_type& fields)
  {
    fields[U"entries"] = value::Function::MakeNative(
      { { U"this", runtime->record_type() } },
      std::make_shared<type::List>(std::make_shared<type::Tuple>(
        std::vector<type::ptr>{ runtime->string_type(), runtime->any_type() })
      ),
      Entries
    );
    fields[U"keys"] = value::Function::MakeNative(
      { { U"this", runtime->record_type() } },
      std::make_shared<type::List>(runtime->string_type()),
      Keys
    );
    fields[U"values"] = value::Function::MakeNative(
      { { U"this", runtime->record_type() } },
      runtime->list_type(),
      Values
    );

    fields[U"+"] = value::Function::MakeNative(
      {
        { U"this", runtime->record_type() },
        { U"other", runtime->record_type() },
      },
      runtime->record_type(),
      Concat
    );
    fields[U"-"] = value::Function::MakeNative(
      {
        { U"this", runtime->record_type() },
        { U"field", runtime->string_type() },
      },
      runtime->record_type(),
      Remove
    );

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
