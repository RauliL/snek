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
#include <climits>
#include <cmath>

#include "snek/interpreter/runtime.hpp"

namespace snek::interpreter::prototype
{
  static inline const value::Number*
  AsNumber(const value::ptr& value)
  {
    return static_cast<value::Number*>(value.get());
  }

  static inline double
  AsFloat(const value::ptr& value)
  {
    return AsNumber(value)->ToFloat();
  }

  static inline std::int64_t
  AsInt(const value::ptr& value)
  {
    return AsNumber(value)->ToInt();
  }

  static int
  DoCompare(const std::vector<value::ptr>& arguments)
  {
    const auto a = AsNumber(arguments[0]);
    const auto b = AsNumber(arguments[1]);

    if (a->kind() == value::Kind::Float || b->kind() == value::Kind::Float)
    {
      const auto x = a->ToFloat();
      const auto y = b->ToFloat();

      return x > y ? 1 : x < y ? -1 : 0;
    } else {
      const auto x = a->ToInt();
      const auto y = b->ToInt();

      return x > y ? 1 : x < y ? -1 : 0;
    }
  }

  template<class FloatOp, class IntOp>
  static value::ptr
  DoOp(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    const auto a = AsNumber(arguments[0]);
    const auto b = AsNumber(arguments[1]);
    const auto result = FloatOp()(a->ToFloat(), b->ToFloat());

    if (
      a->kind() == value::Kind::Int &&
      b->kind() == value::Kind::Int &&
      std::fabs(result) <= static_cast<double>(INT64_MAX)
    )
    {
      // Repeat the operation with full integer precision.
      return runtime.MakeInt(IntOp()(a->ToInt(), b->ToInt()));
    }

    return std::make_shared<value::Float>(result);
  }

  template<class Op>
  static value::ptr
  DoBitOp(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    const auto a = AsNumber(arguments[0]);
    const auto b = AsNumber(arguments[1]);

    return runtime.MakeInt(Op()(a->ToInt(), b->ToInt()));
  }

  /**
   * Number#round(this: Number) => Int
   *
   * Rounds the number to nearest integer value.
   */
  static value::ptr
  Round(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    return runtime.MakeInt(
      static_cast<std::int64_t>(std::round(AsFloat(arguments[0])))
    );
  }

  /**
   * Number#ceil(this: Number) => Int
   *
   * Computes the smallest integer value not less than given number.
   */
  static value::ptr
  Ceil(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    return runtime.MakeInt(
      static_cast<std::int64_t>(std::ceil(AsFloat(arguments[0])))
    );
  }

  /**
   * Number#floor(this: Number) => Int
   *
   * Computes the largest integer value not greater than given number.
   */
  static value::ptr
  Floor(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    return runtime.MakeInt(
      static_cast<std::int64_t>(std::floor(AsFloat(arguments[0])))
    );
  }

  /**
   * Number#+(this: Number, other: Number) => Number
   *
   * Performs addition on the two given numbers.
   */
  static value::ptr
  Add(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    return DoOp<std::plus<double>, std::plus<std::int64_t>>(
      runtime,
      arguments
    );
  }

  /**
   * Number#-(this: Number, other: Number) => Number
   *
   * Performs substraction on the two given numbers.
   */
  static value::ptr
  Sub(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    return DoOp<std::minus<double>, std::minus<std::int64_t>>(
      runtime,
      arguments
    );
  }

  /**
   * Number#*(this: Number, other: Number) => Number
   *
   * Performs multiplication on the two given numbers.
   */
  static value::ptr
  Mul(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    return DoOp<
      std::multiplies<double>,
      std::multiplies<std::int64_t>
    >(runtime, arguments);
  }

  /**
   * Number#/(this: Number, other: Number) => Number
   *
   * Performs division on the two given numbers.
   */
  static value::ptr
  Div(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    return DoOp<std::divides<double>, std::divides<std::int64_t>>(
      runtime,
      arguments
    );
  }

  /**
   * Number#%(this: Number, other: Number) => Int
   *
   * Calculates modulo of the first number with respect to the second number
   * i.e. the remainder after floor division.
   */
  static value::ptr
  Mod(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    const auto a = AsNumber(arguments[0]);
    const auto b = AsNumber(arguments[1]);

    if (a->kind() == value::Kind::Float || b->kind() == value::Kind::Float)
    {
      const auto dividend = a->ToFloat();
      const auto divider = b->ToFloat();
      auto result = std::fmod(dividend, divider);

      if (std::signbit(dividend) != std::signbit(divider))
      {
        result += divider;
      }

      return std::make_shared<value::Float>(result);
    } else {
      const auto dividend = a->ToInt();
      const auto divider = b->ToInt();

      if (divider == 0)
      {
        return std::make_shared<value::Float>(NAN);
      }

      return runtime.MakeInt(dividend % divider);
    }
  }

  /**
   * Number#&(this: Number, other: Number) => Int
   *
   * Performs bitwise and on the two given numbers.
   */
  static value::ptr
  BitwiseAnd(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    return DoBitOp<std::bit_and<std::int64_t>>(runtime, arguments);
  }

  /**
   * Number#|(this: Number, other: Number) => Int
   *
   * Performs bitwise and on the two given numbers.
   */
  static value::ptr
  BitwiseOr(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    return DoBitOp<std::bit_or<std::int64_t>>(runtime, arguments);
  }

  /**
   * Number#^(this: Number, other: Number) => Int
   *
   * Performs bitwise and on the two given numbers.
   */
  static value::ptr
  BitwiseXor(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    return DoBitOp<std::bit_xor<std::int64_t>>(runtime, arguments);
  }

  /**
   * Number#~(this: Number) => Int
   *
   * Flips the bits of the value.
   */
  static value::ptr
  BitwiseNot(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    return runtime.MakeInt(~AsInt(arguments[0]));
  }

  /**
   * Number#<<(this: Number, other: Number) => Int
   *
   * Returns the first value with bits shifted left by the second value.
   */
  static value::ptr
  LeftShift(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    return runtime.MakeInt(AsInt(arguments[0]) << AsInt(arguments[1]));
  }

  /**
   * Number#>>this: Number, other: Number) => Int
   *
   * Returns the first value with bits shifted right by the second value.
   */
  static value::ptr
  RightShift(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    return runtime.MakeInt(AsInt(arguments[0]) >> AsInt(arguments[1]));
  }

  /**
   * Number#<(this: Number, other: Number) => Boolean
   *
   * Returns true if number value is less than the other one.
   */
  static value::ptr
  LessThan(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    return runtime.MakeBoolean(DoCompare(arguments) < 0);
  }

  /**
   * Number#>(this: Number, other: Number) => Boolean
   *
   * Returns true if number value is greater than the other one.
   */
  static value::ptr
  GreaterThan(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    return runtime.MakeBoolean(DoCompare(arguments) > 0);
  }

  /**
   * Number#<=(this: Number, other: Number) => Boolean
   *
   * Returns true if number value is less than the other one or equal.
   */
  static value::ptr
  LessThanOrEqual(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    return runtime.MakeBoolean(DoCompare(arguments) <= 0);
  }

  /**
   * Number#>=(this: Number, other: Number) => Boolean
   *
   * Returns true if number value is greater than the other one or equal.
   */
  static value::ptr
  GreaterThanOrEqual(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    return runtime.MakeBoolean(DoCompare(arguments) >= 0);
  }

  /**
   * Number#+@(this: Number) => Number
   *
   * Returns number value itself.
   */
  static value::ptr
  UnaryPlus(Runtime&, const std::vector<value::ptr>& arguments)
  {
    return arguments[0];
  }

  /**
   * Number#-@(this: Number) => Number
   *
   * Negates number value.
   */
  static value::ptr
  UnaryMinus(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    if (value::IsFloat(arguments[0]))
    {
      return std::make_shared<value::Float>(
        -static_cast<const value::Float*>(arguments[0].get())->value()
      );
    }

    return runtime.MakeInt(
      -static_cast<const value::Int*>(arguments[0].get())->value()
    );
  }

  void
  MakeNumber(const Runtime* runtime, value::Record::container_type& fields)
  {
    fields[U"round"] = value::Function::MakeNative(
      { { U"this", runtime->number_type() } },
      runtime->int_type(),
      Round
    );
    fields[U"ceil"] = value::Function::MakeNative(
      { { U"this", runtime->number_type() } },
      runtime->int_type(),
      Ceil
    );
    fields[U"floor"] = value::Function::MakeNative(
      { { U"this", runtime->number_type() } },
      runtime->int_type(),
      Floor
    );

    fields[U"+"] = value::Function::MakeNative(
      {
        { U"this", runtime->number_type() },
        { U"other", runtime->number_type() },
      },
      runtime->number_type(),
      Add
    );
    fields[U"-"] = value::Function::MakeNative(
      {
        { U"this", runtime->number_type() },
        { U"other", runtime->number_type() },
      },
      runtime->number_type(),
      Sub
    );
    fields[U"*"] = value::Function::MakeNative(
      {
        { U"this", runtime->number_type() },
        { U"other", runtime->number_type() },
      },
      runtime->number_type(),
      Mul
    );
    fields[U"/"] = value::Function::MakeNative(
      {
        { U"this", runtime->number_type() },
        { U"other", runtime->number_type() },
      },
      runtime->number_type(),
      Div
    );
    fields[U"%"] = value::Function::MakeNative(
      {
        { U"this", runtime->number_type() },
        { U"other", runtime->number_type() },
      },
      runtime->number_type(),
      Mod
    );
    fields[U"&"] = value::Function::MakeNative(
      {
        { U"this", runtime->number_type() },
        { U"other", runtime->number_type() },
      },
      runtime->int_type(),
      BitwiseAnd
    );
    fields[U"|"] = value::Function::MakeNative(
      {
        { U"this", runtime->number_type() },
        { U"other", runtime->number_type() },
      },
      runtime->int_type(),
      BitwiseOr
    );
    fields[U"^"] = value::Function::MakeNative(
      {
        { U"this", runtime->number_type() },
        { U"other", runtime->number_type() },
      },
      runtime->int_type(),
      BitwiseXor
    );
    fields[U"~"] = value::Function::MakeNative(
      {
        { U"this", runtime->number_type() },
      },
      runtime->int_type(),
      BitwiseNot
    );
    fields[U"<<"] = value::Function::MakeNative(
      {
        { U"this", runtime->number_type() },
        { U"other", runtime->number_type() },
      },
      runtime->int_type(),
      LeftShift
    );
    fields[U">>"] = value::Function::MakeNative(
      {
        { U"this", runtime->number_type() },
        { U"other", runtime->number_type() },
      },
      runtime->int_type(),
      RightShift
    );
    fields[U"<"] = value::Function::MakeNative(
      {
        { U"this", runtime->number_type() },
        { U"other", runtime->number_type() },
      },
      runtime->boolean_type(),
      LessThan
    );
    fields[U">"] = value::Function::MakeNative(
      {
        { U"this", runtime->number_type() },
        { U"other", runtime->number_type() },
      },
      runtime->boolean_type(),
      GreaterThan
    );
    fields[U"<="] = value::Function::MakeNative(
      {
        { U"this", runtime->number_type() },
        { U"other", runtime->number_type() },
      },
      runtime->boolean_type(),
      LessThanOrEqual
    );
    fields[U">="] = value::Function::MakeNative(
      {
        { U"this", runtime->number_type() },
        { U"other", runtime->number_type() },
      },
      runtime->boolean_type(),
      GreaterThanOrEqual
    );
    fields[U"+@"] = value::Function::MakeNative(
      { { U"this", runtime->number_type() } },
      runtime->number_type(),
      UnaryPlus
    );
    fields[U"-@"] = value::Function::MakeNative(
      { { U"this", runtime->number_type() } },
      runtime->number_type(),
      UnaryMinus
    );
  }
}
