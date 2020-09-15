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
#include <snek/ast/expr/subscript.hpp>
#include <snek/type/base.hpp>
#include <snek/value/record.hpp>
#include <snek/value/str.hpp>

namespace snek::ast::expr
{
  Subscript::Subscript(
    const Position& position,
    const std::shared_ptr<RValue>& record_expression,
    const std::shared_ptr<RValue>& field_expression
  )
    : RValue(position)
    , m_record_expression(record_expression)
    , m_field_expression(field_expression) {}

  RValue::result_type
  Subscript::eval(Interpreter& interpreter, const Scope& scope) const
  {
    const auto record_result = m_record_expression->eval(interpreter, scope);

    if (record_result)
    {
      const auto field_result = m_field_expression->eval(interpreter, scope);

      if (field_result)
      {
        std::shared_ptr<value::Record> record;
        std::shared_ptr<value::Str> field_name;
        value::Record::const_iterator record_field;

        if (record_result.value()->kind() != value::Kind::Record)
        {
          return result_type::error({
            m_record_expression->position(),
            record_result.value()->type(interpreter)->to_string() +
            U" is not an record."
          });
        }
        else if (field_result.value()->kind() != value::Kind::Str)
        {
          return result_type::error({
            m_field_expression->position(),
            field_result.value()->type(interpreter)->to_string() +
            U" is not an string."
          });
        }
        record = std::static_pointer_cast<value::Record>(
          record_result.value()
        );
        field_name = std::static_pointer_cast<value::Str>(
          field_result.value()
        );
        record_field = record->fields().find(field_name->value());
        if (record_field == std::end(record->fields()))
        {
          return result_type::error({
            position(),
            record->type(interpreter)->to_string() +
            U" does not have field `" +
            field_name->value() +
            U"'."
          });
        }
      }

      return field_result;
    }

    return record_result;
  }
}
