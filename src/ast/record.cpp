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
#include <snek/ast/expr/base.hpp>
#include <snek/ast/record.hpp>
#include <snek/scope.hpp>
#include <snek/type/base.hpp>
#include <snek/value/record.hpp>
#include <snek/value/str.hpp>

namespace snek::ast::record
{
  Field::Field(const Position& position)
    : Node(position) {}

  Field::result_type
  Field::assign(Interpreter&, Scope&, const record_type&) const
  {
    return result_type({
      position(),
      U"Cannot assign into TODO."
    });
  }

  Named::Named(
    const Position& position,
    const std::u32string& name,
    const std::shared_ptr<expr::RValue>& value_expression
  )
    : Field(position)
    , m_name(name)
    , m_value_expression(value_expression) {}

  std::u32string
  Named::to_string() const
  {
    return m_name + U": " + m_value_expression->to_string();
  }

  Field::result_type
  Named::eval(
    Interpreter& interpreter,
    const Scope& scope,
    record_type& record
  ) const
  {
    const auto value = m_value_expression->eval(interpreter, scope);

    if (!value)
    {
      return result_type(value.error());
    }
    record[m_name] = value.value();

    return std::nullopt;
  }

  Expr::Expr(
    const Position& position,
    const std::shared_ptr<expr::RValue>& name_expression,
    const std::shared_ptr<expr::RValue>& value_expression
  )
    : Field(position)
    , m_name_expression(name_expression)
    , m_value_expression(value_expression) {}

  std::u32string
  Expr::to_string() const
  {
    return (
      U'[' +
      m_name_expression->to_string() +
      U"]: " +
      m_value_expression->to_string()
    );
  }

  Field::result_type
  Expr::eval(
    Interpreter& interpreter,
    const Scope& scope,
    record_type& record
  ) const
  {
    const auto name = m_name_expression->eval(interpreter, scope);

    if (!name)
    {
      return result_type(name.error());
    }
    else if (name.value()->kind() != value::Kind::Str)
    {
      return result_type({
        m_name_expression->position(),
        U"Expected Str, got " +
        name.value()->type(interpreter)->to_string() +
        U" instead."
      });
    }

    const auto value = m_value_expression->eval(interpreter, scope);

    if (!value)
    {
      return result_type(value.error());
    }

    record[std::static_pointer_cast<value::Str>(name.value())->value()] =
      value.value();

    return std::nullopt;
  }

  Var::Var(const Position& position, const std::u32string& name)
    : Field(position)
    , m_name(name) {}

  Field::result_type
  Var::eval(Interpreter&, const Scope& scope, record_type& record) const
  {
    const auto value = scope.find_variable(m_name);

    if (!value)
    {
      return result_type({
        position(),
        U"Unrecognized identifier `" + m_name + U"'."
      });
    }
    record[m_name] = *value;

    return std::nullopt;
  }

  Field::result_type
  Var::assign(Interpreter&, Scope& scope, const record_type& record) const
  {
    const auto value = record.find(m_name);

    if (value == std::end(record))
    {
      return result_type({
        position(),
        U"Unrecognized identifier `" + m_name + U"'."
      });
    }
    else if (!scope.add_variable(m_name, value->second))
    {
      return result_type({
        position(),
        U"Variable `" +
        m_name +
        U"' has already been defined."
      });
    }

    return std::nullopt;
  }

  Spread::Spread(
    const Position& position,
    const std::shared_ptr<expr::RValue>& expression
  )
    : Field(position)
    , m_expression(expression) {}

  std::u32string
  Spread::to_string() const
  {
    return U"..." + m_expression->to_string();
  }

  Field::result_type
  Spread::eval(
    Interpreter& interpreter,
    const Scope& scope,
    record_type& record
  ) const
  {
    const auto value = m_expression->eval(interpreter, scope);

    if (!value)
    {
      return result_type(value.error());
    }
    else if (value.value()->kind() != value::Kind::Record)
    {
      return result_type({
        m_expression->position(),
        U"Expected record, got " +
        value.value()->type(interpreter)->to_string() +
        U" instead."
      });
    }
    for (const auto& field :
         *std::static_pointer_cast<value::Record>(value.value()))
    {
      record[field.first] = field.second;
    }

    return std::nullopt;
  }
}
