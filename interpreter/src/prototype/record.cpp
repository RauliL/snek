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
#include <unordered_set>

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

    for (const auto& field : record->GetOwnPropertyNames())
    {
      result.push_back(value::List::Make({
        value::String::Make(field),
        *record->GetOwnProperty(field)
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

    for (const auto& field : record->GetOwnPropertyNames())
    {
      result.push_back(value::String::Make(field));
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

    for (const auto& field : record->GetOwnPropertyNames())
    {
      result.push_back(*record->GetOwnProperty(field));
    }

    return value::List::Make(result);
  }

  namespace
  {
    class ConcatRecord final : public value::Record
    {
    public:
      using name_container_type = std::unordered_set<key_type>;

      explicit ConcatRecord(
        const std::shared_ptr<Record>& left,
        const std::shared_ptr<Record>& right
      )
        : m_left(left)
        , m_left_names(FromVector(left->GetOwnPropertyNames()))
        , m_right(right)
        , m_right_names(FromVector(right->GetOwnPropertyNames()))
        , m_all_names(Merge(m_left_names, m_right_names)) {}

      inline size_type
      GetSize() const override
      {
        return m_all_names.size();
      }

      inline std::vector<key_type>
      GetOwnPropertyNames() const override
      {
        return m_all_names;
      }

      std::optional<mapped_type>
      GetOwnProperty(const std::u32string& name) const override
      {
        if (m_right_names.find(name) != std::end(m_right_names))
        {
          return m_right->GetOwnProperty(name);
        }
        else if (m_left_names.find(name) != std::end(m_left_names))
        {
          return m_left->GetOwnProperty(name);
        }

        return nullptr;
      }

    private:
      static inline name_container_type
      FromVector(const std::vector<key_type>& keys)
      {
        return name_container_type(std::begin(keys), std::end(keys));
      }

      static inline std::vector<key_type>
      Merge(const name_container_type& left, const name_container_type& right)
      {
        name_container_type result;

        result.insert(std::begin(left), std::end(left));
        result.insert(std::begin(right), std::end(right));

        return std::vector<key_type>(std::begin(result), std::end(result));
      }

    private:
      const std::shared_ptr<Record> m_left;
      const name_container_type m_left_names;
      const std::shared_ptr<Record> m_right;
      const name_container_type m_right_names;
      const std::vector<key_type> m_all_names;
    };
  }

  /**
   * Record#+(this: Record, other: Record) => Record
   *
   * Combines fields of two records into a new record.
   */
  static value::ptr
  Concat(Runtime&, const std::vector<value::ptr>& arguments)
  {
    return std::make_shared<ConcatRecord>(
      std::static_pointer_cast<value::Record>(arguments[0]),
      std::static_pointer_cast<value::Record>(arguments[1])
    );
  }

  namespace
  {
    class RemoveRecord final : public value::Record
    {
    public:
      explicit RemoveRecord(
        const std::shared_ptr<Record>& record,
        const key_type& removed_name
      )
        : m_record(record)
        , m_removed_name(removed_name) {}

      inline size_type GetSize() const override
      {
        return m_record->GetSize() - 1;
      }

      inline std::vector<key_type>
      GetOwnPropertyNames() const override
      {
        auto names = m_record->GetOwnPropertyNames();
        const auto it = std::find(
          std::begin(names),
          std::end(names),
          m_removed_name
        );

        if (it != std::end(names))
        {
          names.erase(it);
        }

        return names;
      }

      inline std::optional<mapped_type>
      GetOwnProperty(const std::u32string& name) const override
      {
        return name == m_removed_name
          ? std::nullopt
          : m_record->GetOwnProperty(name);
      }

    private:
      const std::shared_ptr<Record> m_record;
      const key_type m_removed_name;
    };
  }

  /**
   * Record#-(this: Record, field: String) => Record
   *
   * Removes field from the record.
   */
  static value::ptr
  Remove(Runtime&, const std::vector<value::ptr>& arguments)
  {
    const auto record = As<value::Record>(arguments[0]);
    const auto key = As<value::String>(arguments[1])->ToString();

    if (record->HasOwnProperty(key))
    {
      return std::make_shared<RemoveRecord>(
        std::static_pointer_cast<value::Record>(arguments[0]),
        key
      );
    }

    return arguments[0];
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
  MakeRecord(
    const Runtime* runtime,
    std::unordered_map<std::u32string, value::ptr>& fields
  )
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
