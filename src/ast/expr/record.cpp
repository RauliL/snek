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
#include <snek/ast/expr/record.hpp>
#include <snek/ast/record.hpp>
#include <snek/type/base.hpp>
#include <snek/value/record.hpp>

namespace snek::ast::expr
{
  Record::Record(
    const Position& position,
    const container_type& fields
  )
    : LValue(position)
    , m_fields(fields) {}

  std::u32string
  Record::to_string() const
  {
    std::u32string result;

    result += U'{';
    for (std::size_t i = 0; i < m_fields.size(); ++i)
    {
      if (i > 0)
      {
        result += U", ";
      }
      result += m_fields[i]->to_string();
    }
    result += U'}';

    return result;
  };

  RValue::result_type
  Record::eval(Interpreter& interpreter, const Scope& scope) const
  {
    snek::value::Record::container_type fields;

    for (const auto& field : m_fields)
    {
      if (const auto error = field->eval(interpreter, scope, fields))
      {
        return result_type::error(*error);
      }
    }

    return result_type::ok(std::make_shared<value::Record>(fields));
  }

  LValue::assign_result_type
  Record::assign(
    Interpreter& interpreter,
    Scope& scope,
    const std::shared_ptr<value::Base>& value
  ) const
  {
    if (value->kind() == value::Kind::Record)
    {
      const auto record =
        std::static_pointer_cast<value::Record>(value)->fields();

      for (const auto& field : m_fields)
      {
        if (const auto error = field->assign(interpreter, scope, record))
        {
          return error;
        }
      }

      return std::nullopt;
    }

    return assign_result_type({
      position(),
      U"Cannot assign " +
      value->type(interpreter)->to_string() +
      U" into an record."
    });
  }
}
