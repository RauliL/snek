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
#include "snek/interpreter/value.hpp"
#include "snek/parser/utils.hpp"

namespace snek::interpreter::value
{
  std::optional<ptr>
  Record::GetOwnProperty(const key_type& name) const
  {
    const auto it = m_fields.find(name);

    if (it != std::end(m_fields))
    {
      return it->second;
    }

    return std::nullopt;
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
      else if (m_fields.size() != that_record->m_fields.size())
      {
        return false;
      }
      for (const auto& field : m_fields)
      {
        const auto it = that_record->m_fields.find(field.first);

        if (
          it == std::end(that_record->m_fields) ||
          !value::Equals(field.second, it->second)
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
    std::u32string result;
    bool first = true;

    for (const auto& field : m_fields)
    {
      if (first)
      {
        first = false;
      } else {
        result.append(U", ");
      }
      result.append(field.first);
      result.append(U": ");
      result.append(value::ToString(field.second));
    }

    return result;
  }

  std::u32string
  Record::ToSource() const
  {
    std::u32string result(1, '{');
    bool first = true;

    for (const auto& field : m_fields)
    {
      if (first)
      {
        first = false;
      } else {
        result.append(U", ");
      }
      result
        .append(
          parser::utils::IsId(field.first)
            ? field.first
            : parser::utils::ToJsonString(field.first))
        .append(U": ")
        .append(value::ToSource(field.second));
    }

    return result.append(1, U'}');
  }
}
