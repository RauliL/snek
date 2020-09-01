/*
 * Copyright (c) 2020, Rauli Laine
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
#include <snek/cst.hpp>
#include <snek/type/record.hpp>
#include <snek/value/record.hpp>

namespace snek::type
{
  Record::Record(const container_type& fields)
    : m_fields(fields) {}

  bool
  Record::matches(const value::Ptr& value) const
  {
    value::RecordPtr value_record;

    if (value->kind() != value::Kind::Record)
    {
      return false;
    }
    value_record = std::static_pointer_cast<value::Record>(value);
    for (const auto& field : m_fields)
    {
      const auto value_field = value_record->fields().find(field.first);

      if (value_field == std::end(value_record->fields()) ||
          !field.second->matches(value_field->second))
      {
        return false;
      }
    }

    return true;
  }

  bool
  Record::matches(const Ptr& type) const
  {
    RecordPtr type_record;

    if (type->kind() != Kind::Record)
    {
      return false;
    }
    type_record = std::static_pointer_cast<Record>(type);
    for (const auto& field : m_fields)
    {
      const auto type_field = type_record->fields().find(field.first);

      if (type_field == std::end(type_record->fields()) ||
          !field.second->matches(type_field->second))
      {
        return false;
      }
    }

    return true;
  }

  std::u32string
  Record::to_string() const
  {
    bool first = true;
    std::u32string result;

    result += U"{";
    for (const auto& field : m_fields)
    {
      if (first)
      {
        first = false;
      } else {
        result += U", ";
      }
      if (cst::is_identifier(field.first))
      {
        result += field.first;
      } else {
        result += cst::stringify(field.first);
      }
      result += U": ";
      result += field.second->to_string();
    }
    result += U"}";

    return result;
  }
}
