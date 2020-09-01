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
#include <snek/ast/expr/field.hpp>
#include <snek/type/base.hpp>
#include <snek/value/record.hpp>

namespace snek::ast::expr
{
  Field::Field(
    const Position& position,
    const std::shared_ptr<RValue>& record_expression,
    const std::u32string& field
  )
    : RValue(position)
    , m_record_expression(record_expression)
    , m_field(field) {}

  RValue::result_type
  Field::eval(Interpreter& interpreter, const Scope& scope) const
  {
    const auto record_result = m_record_expression->eval(interpreter, scope);

    if (record_result)
    {
      const auto& value = record_result.value();
      std::shared_ptr<value::Record> record;
      value::Record::const_iterator record_field;

      if (value->kind() != value::Kind::Record)
      {
        return result_type::error({
          position(),
          value->type(interpreter)->to_string() + U" is not an record."
        });
      }
      record = std::static_pointer_cast<value::Record>(value);
      record_field = record->fields().find(m_field);
      if (record_field == std::end(record->fields()))
      {
        return result_type::error({
          position(),
          record->type(interpreter)->to_string() +
          U" does not have field `" +
          m_field +
          U"'."
        });
      }

      return result_type::ok(record_field->second);
    }

    return record_result;
  }
}
