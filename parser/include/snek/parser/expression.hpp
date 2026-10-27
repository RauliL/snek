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
#pragma once

#include <cstdint>

#include "snek/parser/signature.hpp"

namespace snek::parser
{
  enum class ExpressionKind
  {
    Assign,
    Binary,
    Boolean,
    Call,
    Decrement,
    Float,
    Function,
    Id,
    Increment,
    Int,
    List,
    Null,
    Property,
    Record,
    Spread,
    Subscript,
    String,
    Ternary,
    Unary,
  };

  class Expression : public TypedNode<ExpressionKind>
  {
  public:
    DISALLOW_COPY_AND_ASSIGN(Expression);

    class Assign;
    class Binary;
    class Boolean;
    class Call;
    class Decrement;
    class Float;
    class Function;
    class Id;
    class Increment;
    class Int;
    class List;
    class Null;
    class Property;
    class Record;
    class Spread;
    class Subscript;
    class String;
    class Ternary;
    class Unary;

    explicit Expression(
      const std::optional<Position>& position,
      ExpressionKind kind
    )
      : TypedNode(position, kind) {}

    virtual inline bool IsAssignable() const
    {
      return false;
    }

    static expression_ptr Parse(Lexer& lexer);

    static expression_ptr ParseTernary(Lexer& lexer);

    static expression_ptr ParseFunction(Lexer& lexer);
  };

  class Expression::Assign final : public Expression
  {
  public:
    const expression_ptr variable;
    const expression_ptr value;
    const std::optional<AssignOp> op;

    explicit Assign(
      const std::optional<Position>& position,
      const expression_ptr& variable_,
      const expression_ptr& value_,
      const std::optional<AssignOp>& op_ = std::nullopt
    )
      : Expression(position, ExpressionKind::Assign)
      , variable(variable_)
      , value(value_)
      , op(op_) {}

    std::u32string ToString() const override;
  };

  class Expression::Binary final : public Expression
  {
  public:
    const expression_ptr left;
    const BinaryOp op;
    const expression_ptr right;

    explicit Binary(
      const expression_ptr& left_,
      BinaryOp op_,
      const expression_ptr& right_
    )
      : Expression(left_->position, ExpressionKind::Binary)
      , left(left_)
      , op(op_)
      , right(right_) {}

    std::u32string ToString() const override;
  };

  class Expression::Boolean final : public Expression
  {
  public:
    const bool value;

    explicit Boolean(const std::optional<Position>& position, bool value_)
      : Expression(position, ExpressionKind::Boolean)
      , value(value_) {}

    inline std::u32string ToString() const override
    {
      return value ? U"true" : U"false";
    }
  };

  class Expression::Call final : public Expression
  {
  public:
    const expression_ptr expression;
    const std::vector<expression_ptr> arguments;
    const bool conditional;

    explicit Call(
      const std::optional<Position>& position,
      const expression_ptr& expression_,
      const std::vector<expression_ptr>& arguments_ = {},
      bool conditional_ = false
    )
      : Expression(position, ExpressionKind::Call)
      , expression(expression_)
      , arguments(arguments_)
      , conditional(conditional_) {}

    std::u32string ToString() const override;
  };

  class Expression::Decrement final : public Expression
  {
  public:
    const expression_ptr variable;
    const bool pre;

    explicit Decrement(
      const std::optional<Position>& position,
      const expression_ptr& variable_,
      bool pre_
    )
      : Expression(position, ExpressionKind::Decrement)
      , variable(variable_)
      , pre(pre_) {}

    std::u32string ToString() const override;
  };

  class Expression::Float final : public Expression
  {
  public:
    const double value;

    explicit Float(const std::optional<Position>& position, double value_)
      : Expression(position, ExpressionKind::Float)
      , value(value_) {}

    std::u32string ToString() const override;
  };

  class Expression::Function final : public Expression
  {
  public:
    const Signature signature;
    const statement_ptr body;

    explicit Function(
      const std::optional<Position>& position,
      const Signature& signature_,
      const statement_ptr& body_
    )
      : Expression(position, ExpressionKind::Function)
      , signature(signature_)
      , body(body_) {}

    inline std::u32string ToString() const override
    {
      return signature.ToString();
    }
  };

  class Expression::Id final : public Expression
  {
  public:
    const std::u32string identifier;

    explicit Id(
      const std::optional<Position>& position,
      const std::u32string& identifier_
    )
      : Expression(position, ExpressionKind::Id)
      , identifier(identifier_) {}

    inline bool IsAssignable() const override
    {
      return true;
    }

    inline std::u32string ToString() const override
    {
      return identifier;
    }
  };

  class Expression::Increment final : public Expression
  {
  public:
    const expression_ptr variable;
    const bool pre;

    explicit Increment(
      const std::optional<Position>& position,
      const expression_ptr& variable_,
      bool pre_
    )
      : Expression(position, ExpressionKind::Increment)
      , variable(variable_)
      , pre(pre_) {}

    std::u32string ToString() const override;
  };

  class Expression::Int final : public Expression
  {
  public:
    const std::int64_t value;

    explicit Int(const std::optional<Position>& position, std::int64_t value_)
      : Expression(position, ExpressionKind::Int)
      , value(value_) {}

    std::u32string ToString() const override;
  };

  class Expression::List final : public Expression
  {
  public:
    using container_type = std::vector<element_ptr>;

    const container_type elements;

    explicit List(
      const std::optional<Position>& position,
      const container_type& elements_
    )
      : Expression(position, ExpressionKind::List)
      , elements(elements_) {}

    inline bool IsAssignable() const override
    {
      return true;
    }

    std::u32string ToString() const override;
  };

  class Expression::Null final : public Expression
  {
  public:
    explicit Null(const std::optional<Position>& position = std::nullopt)
      : Expression(position, ExpressionKind::Null) {}

    inline std::u32string ToString() const override
    {
      return U"null";
    }
  };

  class Expression::Property final : public Expression
  {
  public:
    const expression_ptr expression;
    const std::u32string name;
    const bool conditional;

    explicit Property(
      const std::optional<Position>& position,
      const expression_ptr& expression_,
      const std::u32string& name_,
      bool conditional_ = false
    )
      : Expression(position, ExpressionKind::Property)
      , expression(expression_)
      , name(name_)
      , conditional(conditional_) {}

    std::u32string ToString() const override;
  };

  class Expression::Record final : public Expression
  {
  public:
    using value_type = field_ptr;
    using container_type = std::vector<value_type>;

    const container_type fields;

    explicit Record(
      const std::optional<Position>& position,
      const container_type& fields_
    )
      : Expression(position, ExpressionKind::Record)
      , fields(fields_) {}

    bool IsAssignable() const override;

    std::u32string ToString() const override;
  };

  class Expression::Spread final : public Expression
  {
  public:
    const expression_ptr expression;

    explicit Spread(
      const std::optional<Position>& position,
      const expression_ptr& expression_
    )
      : Expression(position, ExpressionKind::Spread)
      , expression(expression_) {}

    inline std::u32string ToString() const override
    {
      return U"..." + expression->ToString();
    }
  };

  class Expression::String final : public Expression
  {
  public:
    const std::u32string value;

    explicit String(
      const std::optional<Position>& position,
      const std::u32string& value_
    )
      : Expression(position, ExpressionKind::String)
      , value(value_) {}

    std::u32string ToString() const override;
  };

  class Expression::Ternary final : public Expression
  {
  public:
    const expression_ptr condition;
    const expression_ptr then_expression;
    const expression_ptr else_expression;

    explicit Ternary(
      const std::optional<Position>& position,
      const expression_ptr& condition_,
      const expression_ptr& then_expression_,
      const expression_ptr& else_expression_
    )
      : Expression(position, ExpressionKind::Ternary)
      , condition(condition_)
      , then_expression(then_expression_)
      , else_expression(else_expression_) {}

    std::u32string ToString() const override;
  };

  class Expression::Subscript final : public Expression
  {
  public:
    const expression_ptr expression;
    const expression_ptr index;
    const bool conditional;

    explicit Subscript(
      const std::optional<Position>& position,
      const expression_ptr& expression_,
      const expression_ptr& index_,
      bool conditional_
    )
      : Expression(position, ExpressionKind::Subscript)
      , expression(expression_)
      , index(index_)
      , conditional(conditional_) {}

    std::u32string ToString() const override;
  };

  class Expression::Unary final : public Expression
  {
  public:

    const UnaryOp op;
    const expression_ptr operand;

    explicit Unary(
      const std::optional<Position>& position,
      UnaryOp op_,
      const expression_ptr& operand_
    )
      : Expression(position, ExpressionKind::Unary)
      , op(op_)
      , operand(operand_) {}

    std::u32string ToString() const override;
  };
}
