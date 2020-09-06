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
#include <snek/ast/expr/unary.hpp>
#include <snek/interpreter.hpp>
#include <snek/value/utils.hpp>

namespace snek::ast::expr
{
  Unary::Unary(
    const Position& position,
    UnaryOperator unary_operator,
    const std::shared_ptr<RValue>& expression
  )
    : RValue(position)
    , m_unary_operator(unary_operator)
    , m_expression(expression) {}

  std::u32string
  Unary::to_string() const
  {
    std::u32string result;

    switch (m_unary_operator)
    {
      case UnaryOperator::Add:
        result += U'+';
        break;

      case UnaryOperator::BitwiseNot:
        result += U'~';
        break;

      case UnaryOperator::Not:
        result += U'!';
        break;

      case UnaryOperator::Sub:
        result += U'~';
        break;
    }
    result += m_expression->to_string();

    return result;
  }

  RValue::result_type
  Unary::eval(Interpreter& interpreter, const Scope& scope) const
  {
    using namespace value::utils;
    const auto result = m_expression->eval(interpreter, scope);

    if (!result)
    {
      return result;
    }
    switch (m_unary_operator)
    {
      case UnaryOperator::Add:
        if (result.value()->kind() == value::Kind::Int)
        {
          return result_type::ok(make_int(
            +std::static_pointer_cast<value::Int>(result.value())->value()
          ));
        }
        else if (result.value()->kind() == value::Kind::Float)
        {
          return result_type::ok(make_float(
            +std::static_pointer_cast<value::Float>(result.value())->value()
          ));
        } else {
          return result_type::error({
            position(),
            U"Unexpected " +
            result.value()->type(interpreter)->to_string() +
            U"; Num required."
          });
        }
        break;

      case UnaryOperator::BitwiseNot:
        if (result.value()->kind() == value::Kind::Int)
        {
          return result_type::ok(make_int(
            ~std::static_pointer_cast<value::Int>(result.value())->value()
          ));
        } else {
          return result_type::error({
            position(),
            U"Unexpected " +
            result.value()->type(interpreter)->to_string() +
            U"; Int required."
          });
        }
        break;

      case UnaryOperator::Not:
        if (result.value()->kind() == value::Kind::Bool)
        {
          return result_type::ok(interpreter.bool_value(
            !std::static_pointer_cast<value::Bool>(result.value())
          ));
        } else {
          return result_type::error({
            position(),
            U"Unexpected " +
            result.value()->type(interpreter)->to_string() +
            U"; Bool required."
          });
        }
        break;

      case UnaryOperator::Sub:
        if (result.value()->kind() == value::Kind::Int)
        {
          return result_type::ok(make_int(
            -std::static_pointer_cast<value::Int>(result.value())->value()
          ));
        }
        else if (result.value()->kind() == value::Kind::Float)
        {
          return result_type::ok(make_float(
            -std::static_pointer_cast<value::Float>(result.value())->value()
          ));
        } else {
          return result_type::error({
            position(),
            U"Unexpected " +
            result.value()->type(interpreter)->to_string() +
            U"; Num required."
          });
        }
        break;
    }

    return result_type::error({
      position(),
      U"Unrecognized unary operator."
    });
  }
}
