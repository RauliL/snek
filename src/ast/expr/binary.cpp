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
#include <cmath>
#include <limits>

#include <snek/ast/expr/binary.hpp>
#include <snek/interpreter.hpp>
#include <snek/value/utils.hpp>

namespace snek::ast::expr
{
  using namespace snek::value::utils;

  static const std::unordered_map<BinaryOperator, std::u32string> mapping =
  {
    { BinaryOperator::Add, U"+" },
    { BinaryOperator::Sub, U"-" },
    { BinaryOperator::Mul, U"*" },
    { BinaryOperator::Div, U"/" },
    { BinaryOperator::Mod, U"%" },
    { BinaryOperator::Eq, U"==" },
    { BinaryOperator::Ne, U"!=" },
    { BinaryOperator::Lt, U"<" },
    { BinaryOperator::Gt, U">" },
    { BinaryOperator::Lte, U"<=" },
    { BinaryOperator::Gte, U">=" },
  };

  Binary::Binary(
    const Position& position,
    const std::shared_ptr<RValue>& left_expression,
    BinaryOperator binary_operator,
    const std::shared_ptr<RValue>& right_expression
  )
    : RValue(position)
    , m_left_expression(left_expression)
    , m_binary_operator(binary_operator)
    , m_right_expression(right_expression) {}

  static inline bool
  is_num_kind(value::Kind kind)
  {
    return kind == value::Kind::Int || kind == value::Kind::Float;
  }

  static inline value::Float::value_type
  to_float(const value::Ptr& value)
  {
    if (value->kind() == value::Kind::Float)
    {
      return std::static_pointer_cast<value::Float>(value)->value();
    }

    return static_cast<value::Float::value_type>(
      std::static_pointer_cast<value::Int>(value)->value()
    );
  }

  static inline value::Int::value_type
  to_int(const value::Ptr& value)
  {
    if (value->kind() == value::Kind::Float)
    {
      const auto f = std::static_pointer_cast<value::Float>(value)->value();

      return static_cast<value::Int::value_type>(std::round(f));
    }

    return std::static_pointer_cast<value::Int>(value)->value();
  }

  static RValue::result_type
  op_add(
    const Position& position,
    const value::Ptr& left,
    const value::Ptr& right,
    const Interpreter& interpreter
  )
  {
    const auto left_kind = left->kind();
    const auto right_kind = right->kind();

    if (is_num_kind(left_kind) && is_num_kind(right_kind))
    {
      if (left_kind == value::Kind::Float || right_kind == value::Kind::Float)
      {
        return RValue::result_type::ok(make_float(
          to_float(left) + to_float(right)
        ));
      } else {
        return RValue::result_type::ok(make_int(to_int(left) + to_int(right)));
      }
    }
    else if (left_kind == value::Kind::Str && right_kind == value::Kind::Str)
    {
      return RValue::result_type::ok(make_str(
        std::static_pointer_cast<value::Str>(left)->value() +
        std::static_pointer_cast<value::Str>(right)->value()
      ));
    }

    return RValue::result_type::error({
      position,
      U"Cannot add " +
      left->type(interpreter)->to_string() +
      U" to " +
      right->type(interpreter)->to_string()
    });
  }

  static RValue::result_type
  op_sub(
    const Position& position,
    const value::Ptr& left,
    const value::Ptr& right,
    const Interpreter& interpreter
  )
  {
    const auto left_kind = left->kind();
    const auto right_kind = right->kind();

    if (is_num_kind(left_kind) && is_num_kind(right_kind))
    {
      if (left_kind == value::Kind::Float || right_kind == value::Kind::Float)
      {
        return RValue::result_type::ok(make_float(
          to_float(left) - to_float(right)
        ));
      } else {
        return RValue::result_type::ok(make_int(to_int(left) - to_int(right)));
      }
    }

    return RValue::result_type::error({
      position,
      U"Cannot substract " +
      left->type(interpreter)->to_string() +
      U" from " +
      right->type(interpreter)->to_string()
    });
  }

  static RValue::result_type
  op_mul(
    const Position& position,
    const value::Ptr& left,
    const value::Ptr& right,
    const Interpreter& interpreter
  )
  {
    const auto left_kind = left->kind();
    const auto right_kind = right->kind();

    if (is_num_kind(left_kind) && is_num_kind(right_kind))
    {
      if (left_kind == value::Kind::Float || right_kind == value::Kind::Float)
      {
        return RValue::result_type::ok(make_float(
          to_float(left) * to_float(right)
        ));
      } else {
        return RValue::result_type::ok(make_int(to_int(left) * to_int(right)));
      }
    }

    return RValue::result_type::error({
      position,
      U"Cannot multiply " +
      left->type(interpreter)->to_string() +
      U" with " +
      right->type(interpreter)->to_string()
    });
  }

  static RValue::result_type
  op_div(
    const Position& position,
    const value::Ptr& left,
    const value::Ptr& right,
    const Interpreter& interpreter
  )
  {
    const auto left_kind = left->kind();
    const auto right_kind = right->kind();

    if (is_num_kind(left_kind) && is_num_kind(right_kind))
    {
      if (left_kind == value::Kind::Float || right_kind == value::Kind::Float)
      {
        return RValue::result_type::ok(make_float(
          to_float(left) / to_float(right)
        ));
      } else {
        const auto a = to_int(left);
        const auto b = to_int(right);

        if (b == 0)
        {
          return RValue::result_type::ok(make_float(
            std::numeric_limits<value::Float::value_type>::infinity()
          ));
        }

        return RValue::result_type::ok(make_int(a / b));
      }
    }

    return RValue::result_type::error({
      position,
      U"Cannot divide " +
      left->type(interpreter)->to_string() +
      U" with " +
      right->type(interpreter)->to_string()
    });
  }

  static RValue::result_type
  op_mod(
    const Position& position,
    const value::Ptr& left,
    const value::Ptr& right,
    const Interpreter& interpreter
  )
  {
    const auto left_kind = left->kind();
    const auto right_kind = right->kind();

    if (is_num_kind(left_kind) && is_num_kind(right_kind))
    {
      if (left_kind == value::Kind::Float || right_kind == value::Kind::Float)
      {
        const auto a = to_float(left);
        const auto b = to_float(right);

        if (b == 0.0)
        {
          return RValue::result_type::error({
            position,
            U"Division by zero."
          });
        }

        return RValue::result_type::ok(make_float(std::fmod(a, b)));
      } else {
        const auto a = to_int(left);
        const auto b = to_int(right);

        if (b == 0)
        {
          return RValue::result_type::error({
            position,
            U"Division by zero."
          });
        }

        return RValue::result_type::ok(make_int(a / b));
      }
    }

    return RValue::result_type::error({
      position,
      U"Cannot modulo " +
      left->type(interpreter)->to_string() +
      U" with " +
      right->type(interpreter)->to_string()
    });
  }

  static RValue::result_type
  op_eq(
    const value::Ptr& left,
    const value::Ptr& right,
    const Interpreter& interpreter
  )
  {
    return RValue::result_type::ok(
      interpreter.bool_value(left->equals(right))
    );
  }

  static RValue::result_type
  op_ne(
    const value::Ptr& left,
    const value::Ptr& right,
    const Interpreter& interpreter
  )
  {
    return RValue::result_type::ok(
      interpreter.bool_value(!left->equals(right))
    );
  }

  static RValue::result_type
  op_lt(
    const Position& position,
    const value::Ptr& left,
    const value::Ptr& right,
    const Interpreter& interpreter
  )
  {
    const auto left_kind = left->kind();
    const auto right_kind = right->kind();

    if (is_num_kind(left_kind) && is_num_kind(right_kind))
    {
      bool result;

      if (left_kind == value::Kind::Float || right_kind == value::Kind::Float)
      {
        result = to_float(left) < to_float(right);
      } else {
        result = to_int(left) < to_int(right);
      }

      return RValue::result_type::ok(interpreter.bool_value(result));
    }

    return RValue::result_type::error({
      position,
      U"Cannot compare " +
      left->type(interpreter)->to_string() +
      U" against " +
      right->type(interpreter)->to_string()
    });
  }

  static RValue::result_type
  op_gt(
    const Position& position,
    const value::Ptr& left,
    const value::Ptr& right,
    const Interpreter& interpreter
  )
  {
    const auto left_kind = left->kind();
    const auto right_kind = right->kind();

    if (is_num_kind(left_kind) && is_num_kind(right_kind))
    {
      bool result;

      if (left_kind == value::Kind::Float || right_kind == value::Kind::Float)
      {
        result = to_float(left) > to_float(right);
      } else {
        result = to_int(left) > to_int(right);
      }

      return RValue::result_type::ok(interpreter.bool_value(result));
    }

    return RValue::result_type::error({
      position,
      U"Cannot compare " +
      left->type(interpreter)->to_string() +
      U" against " +
      right->type(interpreter)->to_string()
    });
  }

  static RValue::result_type
  op_lte(
    const Position& position,
    const value::Ptr& left,
    const value::Ptr& right,
    const Interpreter& interpreter
  )
  {
    const auto left_kind = left->kind();
    const auto right_kind = right->kind();

    if (is_num_kind(left_kind) && is_num_kind(right_kind))
    {
      bool result;

      if (left_kind == value::Kind::Float || right_kind == value::Kind::Float)
      {
        result = to_float(left) <= to_float(right);
      } else {
        result = to_int(left) <= to_int(right);
      }

      return RValue::result_type::ok(interpreter.bool_value(result));
    }

    return RValue::result_type::error({
      position,
      U"Cannot compare " +
      left->type(interpreter)->to_string() +
      U" against " +
      right->type(interpreter)->to_string()
    });
  }

  static RValue::result_type
  op_gte(
    const Position& position,
    const value::Ptr& left,
    const value::Ptr& right,
    const Interpreter& interpreter
  )
  {
    const auto left_kind = left->kind();
    const auto right_kind = right->kind();

    if (is_num_kind(left_kind) && is_num_kind(right_kind))
    {
      bool result;

      if (left_kind == value::Kind::Float || right_kind == value::Kind::Float)
      {
        result = to_float(left) >= to_float(right);
      } else {
        result = to_int(left) >= to_int(right);
      }

      return RValue::result_type::ok(interpreter.bool_value(result));
    }

    return RValue::result_type::error({
      position,
      U"Cannot compare " +
      left->type(interpreter)->to_string() +
      U" against " +
      right->type(interpreter)->to_string()
    });
  }

  std::u32string
  Binary::to_string() const
  {
    const auto entry = mapping.find(m_binary_operator);
    std::u32string result;

    result += m_left_expression->to_string();
    result += U' ';
    if (entry != std::end(mapping))
    {
      result += entry->second;
    } else {
      result += U"unknown";
    }
    result += U' ';
    result += m_right_expression->to_string();

    return result;
  }

  RValue::result_type
  Binary::eval(Interpreter& interpreter, const Scope& scope) const
  {
    const auto left = m_left_expression->eval(interpreter, scope);

    if (!left)
    {
      return left;
    }

    const auto right = m_right_expression->eval(interpreter, scope);

    if (!right)
    {
      return right;
    }

    switch (m_binary_operator)
    {
      case BinaryOperator::Add:
        return op_add(position(), left.value(), right.value(), interpreter);

      case BinaryOperator::Sub:
        return op_sub(position(), left.value(), right.value(), interpreter);

      case BinaryOperator::Mul:
        return op_mul(position(), left.value(), right.value(), interpreter);

      case BinaryOperator::Div:
        return op_div(position(), left.value(), right.value(), interpreter);

      case BinaryOperator::Mod:
        return op_mod(position(), left.value(), right.value(), interpreter);

      case BinaryOperator::Eq:
        return op_eq(left.value(), right.value(), interpreter);

      case BinaryOperator::Ne:
        return op_ne(left.value(), right.value(), interpreter);

      case BinaryOperator::Lt:
        return op_lt(position(), left.value(), right.value(), interpreter);

      case BinaryOperator::Gt:
        return op_gt(position(), left.value(), right.value(), interpreter);

      case BinaryOperator::Lte:
        return op_lte(position(), left.value(), right.value(), interpreter);

      case BinaryOperator::Gte:
        return op_gte(position(), left.value(), right.value(), interpreter);
    }

    return result_type::error({
      position(),
      U"Unrecognized binary operator."
    });
  }
}
