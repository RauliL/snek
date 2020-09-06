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
#include <snek/ast/type/record.hpp>
#include <snek/type/record.hpp>

namespace snek::ast::type
{
  Record::Record(
    const Position& position,
    const container_type& fields
  )
    : Base(position)
    , m_fields(fields) {}

  std::u32string
  Record::to_string() const
  {
    std::u32string result;
    bool first = true;

    result += U'{';
    for (const auto& field : m_fields)
    {
      if (first)
      {
        first = false;
      } else {
        result += U", ";
      }
      result += field.first;
      result += U": ";
      result += field.second->to_string();
    }
    result += U'}';

    return result;
  }

  Base::result_type
  Record::eval(const Interpreter& interpreter, const Scope& scope) const
  {
    snek::type::Record::container_type fields;

    for (const auto& field : m_fields)
    {
      const auto type = field.second->eval(interpreter, scope);

      if (!type)
      {
        return type;
      }
      fields[field.first] = type.value();
    }

    return result_type::ok(std::make_shared<snek::type::Record>(fields));
  }
}
