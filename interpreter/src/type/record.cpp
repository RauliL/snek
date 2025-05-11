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
#include "snek/parser/utils.hpp"

#include "./utils.hpp"

namespace snek::interpreter::type
{
  bool
  Record::Accepts(const Runtime& runtime, const value::ptr& value) const
  {
    if (!value::IsRecord(value))
    {
      return false;
    }
    for (const auto& field : m_fields)
    {
      if (const auto property = value::GetProperty(runtime, value, field.first))
      {
        if (!field.second->Accepts(runtime, *property))
        {
          return false;
        }
      } else {
        return false;
      }
    }

    return true;
  }

  bool
  Record::Accepts(const ptr& that) const
  {
    if (!that || this == that.get())
    {
      return true;
    }
    else if (that->kind() == Kind::Record)
    {
      const auto record = utils::As<Record>(that);

      for (const auto& field : m_fields)
      {
        const auto it = record->m_fields.find(field.first);

        if (
          it == std::end(record->m_fields) ||
          !field.second->Accepts(it->second)
        )
        {
          return false;
        }

        return true;
      }
    }
    else if (that->kind() == Kind::Builtin)
    {
      return utils::As<Builtin>(that)->builtin_kind() == BuiltinKind::Record;
    }

    return false;
  }

  std::u32string
  Record::ToString() const
  {
    bool first = true;
    std::u32string result(1, U'{');

    for (const auto& field : m_fields)
    {
      if (first)
      {
        first = false;
      } else {
        result.append(U", ");
      }
      if (parser::utils::IsId(field.first))
      {
        result.append(field.first);
      } else {
        result.append(parser::utils::ToJsonString(field.first));
      }
      result.append(U": ");
      result.append(field.second->ToString());
    }
    result.append(1, U'}');

    return result;
  }
}
