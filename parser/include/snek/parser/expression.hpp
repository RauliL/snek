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

#include "snek/parser/parameter.hpp"

namespace snek::parser::element
{
  class Base;

  using ptr = std::shared_ptr<Base>;
}

namespace snek::parser::field
{
  class Base;

  using ptr = std::shared_ptr<Base>;
}

namespace snek::parser::statement
{
  class Base;

  using ptr = std::shared_ptr<Base>;
}

namespace snek::parser::type
{
  class Base;

  using ptr = std::shared_ptr<Base>;
}

namespace snek::parser::expression
{
  enum class Kind
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

  class Base : public Node
  {
  public:
    DISALLOW_COPY_AND_ASSIGN(Base);

    explicit Base(const std::optional<Position>& position)
      : Node(position) {}

    virtual Kind kind() const = 0;

    virtual inline bool IsAssignable() const
    {
      return false;
    }
  };

  using ptr = std::shared_ptr<Base>;

  ptr Parse(Lexer& lexer);

  ptr ParseTernary(Lexer& lexer);

  class Assign final : public Base
  {
  public:
    enum class Operator
    {
      Add = static_cast<int>(Token::Kind::AssignAdd),
      Sub = static_cast<int>(Token::Kind::AssignSub),
      Mul = static_cast<int>(Token::Kind::AssignMul),
      Div = static_cast<int>(Token::Kind::AssignDiv),
      Mod = static_cast<int>(Token::Kind::AssignMod),
      BitwiseAnd = static_cast<int>(Token::Kind::AssignBitwiseAnd),
      BitwiseOr = static_cast<int>(Token::Kind::AssignBitwiseOr),
      BitwiseXor = static_cast<int>(Token::Kind::AssignBitwiseXor),
      LeftShift = static_cast<int>(Token::Kind::AssignLeftShift),
      RightShift = static_cast<int>(Token::Kind::AssignRightShift),
      LogicalAnd = static_cast<int>(Token::Kind::AssignLogicalAnd),
      LogicalOr = static_cast<int>(Token::Kind::AssignLogicalOr),
      NullCoalescing = static_cast<int>(Token::Kind::AssignNullCoalescing),
    };

    const ptr variable;
    const ptr value;
    const std::optional<Operator> op;

    explicit Assign(
      const std::optional<Position>& position,
      const ptr& variable_,
      const ptr& value_,
      const std::optional<Operator>& op_ = std::nullopt
    )
      : Base(position)
      , variable(variable_)
      , value(value_)
      , op(op_) {}

    inline Kind kind() const override
    {
      return Kind::Assign;
    }

    static std::u32string ToString(Operator op);

    std::u32string ToString() const override;
  };

  class Binary final : public Base
  {
  public:
    enum class Operator
    {
      Add = static_cast<int>(Token::Kind::Add),
      Sub = static_cast<int>(Token::Kind::Sub),
      Mul = static_cast<int>(Token::Kind::Mul),
      Div = static_cast<int>(Token::Kind::Div),
      Mod = static_cast<int>(Token::Kind::Mod),
      BitwiseAnd = static_cast<int>(Token::Kind::BitwiseAnd),
      BitwiseOr = static_cast<int>(Token::Kind::BitwiseOr),
      BitwiseXor = static_cast<int>(Token::Kind::BitwiseXor),
      Equal = static_cast<int>(Token::Kind::Equal),
      NotEqual = static_cast<int>(Token::Kind::NotEqual),
      LessThan = static_cast<int>(Token::Kind::LessThan),
      GreaterThan = static_cast<int>(Token::Kind::GreaterThan),
      LessThanEqual = static_cast<int>(Token::Kind::LessThanEqual),
      GreaterThanEqual = static_cast<int>(Token::Kind::GreaterThanEqual),
      LeftShift = static_cast<int>(Token::Kind::LeftShift),
      RightShift = static_cast<int>(Token::Kind::RightShift),
      LogicalAnd = static_cast<int>(Token::Kind::LogicalAnd),
      LogicalOr = static_cast<int>(Token::Kind::LogicalOr),
      NullCoalescing = static_cast<int>(Token::Kind::NullCoalescing),
    };

    const ptr left;
    const Operator op;
    const ptr right;

    explicit Binary(const ptr& left_, Operator op_, const ptr& right_)
      : Base(left_->position)
      , left(left_)
      , op(op_)
      , right(right_) {}

    inline Kind kind() const override
    {
      return Kind::Binary;
    }

    static std::u32string ToString(Operator op);

    std::u32string ToString() const override;
  };

  class Boolean final : public Base
  {
  public:
    const bool value;

    explicit Boolean(const std::optional<Position>& position, bool value_)
      : Base(position)
      , value(value_) {}

    inline Kind kind() const override
    {
      return Kind::Boolean;
    }

    inline std::u32string ToString() const override
    {
      return value ? U"true" : U"false";
    }
  };

  class Call final : public Base
  {
  public:
    const ptr expression;
    const std::vector<ptr> arguments;
    const bool conditional;

    explicit Call(
      const std::optional<Position>& position,
      const ptr& expression_,
      const std::vector<ptr>& arguments_ = {},
      bool conditional_ = false
    )
      : Base(position)
      , expression(expression_)
      , arguments(arguments_)
      , conditional(conditional_) {}

    inline Kind kind() const override
    {
      return Kind::Call;
    }

    std::u32string ToString() const override;
  };

  class Decrement final : public Base
  {
  public:
    const ptr variable;
    const bool pre;

    explicit Decrement(
      const std::optional<Position>& position,
      const ptr& variable_,
      bool pre_
    )
      : Base(position)
      , variable(variable_)
      , pre(pre_) {}

    inline Kind kind() const override
    {
      return Kind::Decrement;
    }

    std::u32string ToString() const override;
  };

  class Float final : public Base
  {
  public:
    const double value;

    explicit Float(const std::optional<Position>& position, double value_)
      : Base(position)
      , value(value_) {}

    inline Kind kind() const override
    {
      return Kind::Float;
    }

    std::u32string ToString() const override;
  };

  class Function final : public Base
  {
  public:
    const std::vector<Parameter> parameters;
    const type::ptr return_type;
    const statement::ptr body;

    explicit Function(
      const std::optional<Position>& position,
      const std::vector<Parameter>& parameters_,
      const type::ptr& return_type_,
      const statement::ptr& body_
    )
      : Base(position)
      , parameters(parameters_)
      , return_type(return_type_)
      , body(body_) {}

    inline Kind kind() const override
    {
      return Kind::Function;
    }

    std::u32string ToString() const override;
  };

  class Id final : public Base
  {
  public:
    const std::u32string identifier;

    explicit Id(
      const std::optional<Position>& position,
      const std::u32string& identifier_
    )
      : Base(position)
      , identifier(identifier_) {}

    inline Kind kind() const override
    {
      return Kind::Id;
    }

    inline bool IsAssignable() const override
    {
      return true;
    }

    inline std::u32string ToString() const override
    {
      return identifier;
    }
  };

  class Increment final : public Base
  {
  public:
    const ptr variable;
    const bool pre;

    explicit Increment(
      const std::optional<Position>& position,
      const ptr& variable_,
      bool pre_
    )
      : Base(position)
      , variable(variable_)
      , pre(pre_) {}

    inline Kind kind() const override
    {
      return Kind::Increment;
    }

    std::u32string ToString() const override;
  };

  class Int final : public Base
  {
  public:
    const std::int64_t value;

    explicit Int(const std::optional<Position>& position, std::int64_t value_)
      : Base(position)
      , value(value_) {}

    inline Kind kind() const override
    {
      return Kind::Int;
    }

    std::u32string ToString() const override;
  };

  class List final : public Base
  {
  public:
    using container_type = std::vector<element::ptr>;

    const container_type elements;

    explicit List(
      const std::optional<Position>& position,
      const container_type& elements_
    )
      : Base(position)
      , elements(elements_) {}

    inline Kind kind() const override
    {
      return Kind::List;
    }

    inline bool IsAssignable() const override
    {
      return true;
    }

    std::u32string ToString() const override;
  };

  class Null final : public Base
  {
  public:
    explicit Null(const std::optional<Position>& position = std::nullopt)
      : Base(position) {}

    inline Kind kind() const override
    {
      return Kind::Null;
    }

    inline std::u32string ToString() const override
    {
      return U"null";
    }
  };

  class Property final : public Base
  {
  public:
    const ptr expression;
    const std::u32string name;
    const bool conditional;

    explicit Property(
      const std::optional<Position>& position,
      const ptr& expression_,
      const std::u32string& name_,
      bool conditional_ = false
    )
      : Base(position)
      , expression(expression_)
      , name(name_)
      , conditional(conditional_) {}

    inline Kind kind() const override
    {
      return Kind::Property;
    }

    std::u32string ToString() const override;
  };

  class Record final : public Base
  {
  public:
    using container_type = std::vector<field::ptr>;

    const container_type fields;

    explicit Record(
      const std::optional<Position>& position,
      const container_type& fields_
    )
      : Base(position)
      , fields(fields_) {}

    inline Kind kind() const override
    {
      return Kind::Record;
    }

    bool IsAssignable() const override;

    std::u32string ToString() const override;
  };

  class Spread final : public Base
  {
  public:
    const ptr expression;

    explicit Spread(
      const std::optional<Position>& position,
      const ptr& expression_
    )
      : Base(position)
      , expression(expression_) {}

    inline Kind kind() const override
    {
      return Kind::Spread;
    }

    inline std::u32string ToString() const override
    {
      return U"..." + expression->ToString();
    }
  };

  class String final : public Base
  {
  public:
    const std::u32string value;

    explicit String(
      const std::optional<Position>& position,
      const std::u32string& value_
    )
      : Base(position)
      , value(value_) {}

    inline Kind kind() const override
    {
      return Kind::String;
    }

    std::u32string ToString() const override;
  };

  class Ternary final : public Base
  {
  public:
    const ptr condition;
    const ptr then_expression;
    const ptr else_expression;

    explicit Ternary(
      const std::optional<Position>& position,
      const ptr& condition_,
      const ptr& then_expression_,
      const ptr& else_expression_
    )
      : Base(position)
      , condition(condition_)
      , then_expression(then_expression_)
      , else_expression(else_expression_) {}

    inline Kind kind() const override
    {
      return Kind::Ternary;
    }

    std::u32string ToString() const override;
  };

  class Subscript final : public Base
  {
  public:
    const ptr expression;
    const ptr index;
    const bool conditional;

    explicit Subscript(
      const std::optional<Position>& position,
      const ptr& expression_,
      const ptr& index_,
      bool conditional_
    )
      : Base(position)
      , expression(expression_)
      , index(index_)
      , conditional(conditional_) {}

    inline Kind kind() const override
    {
      return Kind::Subscript;
    }

    std::u32string ToString() const override;
  };

  class Unary final : public Base
  {
  public:
    enum class Operator
    {
      Not = static_cast<int>(Token::Kind::Not),
      Add = static_cast<int>(Token::Kind::Add),
      Sub = static_cast<int>(Token::Kind::Sub),
      BitwiseNot = static_cast<int>(Token::Kind::BitwiseNot),
    };

    static std::u32string GetMethodName(Operator op);

    const Operator op;
    const ptr operand;

    explicit Unary(
      const std::optional<Position>& position,
      Operator op_,
      const ptr& operand_
    )
      : Base(position)
      , op(op_)
      , operand(operand_) {}

    inline Kind kind() const override
    {
      return Kind::Unary;
    }

    static std::u32string ToString(Operator op);

    std::u32string ToString() const override;
  };
}
