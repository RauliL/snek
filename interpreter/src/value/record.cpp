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

#include "snek/interpreter/value.hpp"
#include "snek/parser/utils.hpp"

namespace snek::interpreter::value
{
  namespace
  {
    class MapRecord final : public Record
    {
    public:
      explicit MapRecord(const container_type& fields)
        : m_fields(fields) {}

      inline size_type
      GetSize() const override
      {
        return m_fields.size();
      }

      std::optional<ptr>
      GetOwnProperty(const key_type& name) const override
      {
        const auto it = m_fields.find(name);

        if (it != std::end(m_fields))
        {
          return it->second;
        }

        return std::nullopt;
      }

      std::vector<key_type>
      GetOwnPropertyNames() const override
      {
        std::vector<key_type> result;

        result.reserve(m_fields.size());
        for (const auto& field : m_fields)
        {
          result.push_back(field.first);
        }

        return result;
      }

    private:
      const container_type m_fields;
    };

    class MergeRecord final : public Record
    {
    public:
      explicit MergeRecord(const record_ptr& first, const record_ptr& second)
        : m_first(first)
        , m_second(second)
        , m_own_property_names(MakeOwnPropertyNames(first, second)) {}

      inline size_type
      GetSize() const override
      {
        return GetOwnPropertyNames().size();
      }

      std::optional<ptr>
      GetOwnProperty(const key_type& name) const override
      {
        if (const auto value = m_second->GetOwnProperty(name))
        {
          return value;
        }

        return m_first->GetOwnProperty(name);
      }

      std::vector<key_type>
      GetOwnPropertyNames() const override
      {
        return m_own_property_names;
      }

    private:
      static std::vector<key_type>
      MakeOwnPropertyNames(const record_ptr& first, const record_ptr& second)
      {
        const auto first_keys = first->GetOwnPropertyNames();
        const auto second_keys = second->GetOwnPropertyNames();
        std::unordered_set<key_type> result(
          std::begin(first_keys),
          std::end(first_keys)
        );

        result.insert(std::begin(second_keys), std::end(second_keys));

        return std::vector<key_type>(std::begin(result), std::end(result));
      }

    private:
      const record_ptr m_first;
      const record_ptr m_second;
      const std::vector<key_type> m_own_property_names;
    };
  }

  record_ptr
  Record::Make(const container_type& fields)
  {
    return std::make_shared<MapRecord>(fields);
  }

  record_ptr
  Record::Merge(const record_ptr& first, const record_ptr& second)
  {
    return std::make_shared<MergeRecord>(first, second);
  }

  void
  Record::ForEach(
    const std::function<void(
      const key_type&,
      const mapped_type&
    )>& callback
  ) const
  {
    for (const auto& field : GetOwnPropertyNames())
    {
      callback(field, *GetOwnProperty(field));
    }
  }

  bool
  Record::Equals(const Base& that) const
  {
    if (that.kind() == Kind::Record)
    {
      const auto that_record = static_cast<const Record*>(&that);

      if (this == that_record)
      {
        return true;
      }
      else if (GetSize() != that_record->GetSize())
      {
        return false;
      }
      for (const auto& field_name : GetOwnPropertyNames())
      {
        const auto property = that_record->GetOwnProperty(field_name);

        if (
          !property ||
          !value::Equals(*GetOwnProperty(field_name), *property)
        )
        {
          return false;
        }
      }

      return true;
    }

    return false;
  }

  std::u32string
  Record::ToString() const
  {
    const auto property_names = GetOwnPropertyNames();

    return parser::utils::Join<
      std::u32string,
      std::vector<std::u32string>::const_iterator
    >(
      std::begin(property_names),
      std::end(property_names),
      [this](const auto& field_name)
      {
        return field_name
          + U": "
          + value::ToString(*GetOwnProperty(field_name));
      }
    );
  }

  std::u32string
  Record::ToSource() const
  {
    const auto property_names = GetOwnPropertyNames();

    return parser::utils::Join<
      std::u32string,
      std::vector<std::u32string>::const_iterator
    >(
      std::begin(property_names),
      std::end(property_names),
      [this](const auto& field_name)
      {
        return (
          parser::utils::IsId(field_name)
            ? field_name
            : parser::utils::ToJsonString(field_name)
        )
          + U": "
          + value::ToSource(*GetOwnProperty(field_name));
      },
      U'{',
      U'}'
    );
  }
}
