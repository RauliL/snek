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
#include <memory>
#include <vector>

#include "snek/parser/lexer.hpp"
#include "snek/parser/node.hpp"

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

namespace snek::parser::parameter
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
    Float,
    Function,
    Id,
    Int,
    List,
    Null,
    Property,
    Record,
    Subscript,
    String,
    Ternary,
    Unary,
  };

  class Base : public Node
  {
  public:
    DISALLOW_COPY_AND_ASSIGN(Base);

    explicit Base(const Position& position)
      : Node(position) {}

    virtual Kind kind() const = 0;

    virtual inline bool IsAssignable() const
    {
      return false;
    }
  };

  using ptr = std::shared_ptr<Base>;

  ptr Parse(Lexer& lexer);

  class Assign final : public Base
  {
  public:
    explicit Assign(
      const Position& position,
      const ptr& variable,
      const ptr& value,
      const std::optional<Token::Kind>& op = std::nullopt
    )
      : Base(position)
      , m_variable(variable)
      , m_value(value)
      , m_op(op) {}

    inline Kind kind() const override
    {
      return Kind::Assign;
    }

    inline const ptr& variable() const
    {
      return m_variable;
    }

    inline const ptr& value() const
    {
      return m_value;
    }

    inline const std::optional<Token::Kind>& op() const
    {
      return m_op;
    }

    static std::u32string ToString(Token::Kind op);

    std::u32string ToString() const override;

  private:
    const ptr m_variable;
    const ptr m_value;
    const std::optional<Token::Kind> m_op;
  };

  class Binary final : public Base
  {
  public:
    explicit Binary(const ptr& left, Token::Kind op, const ptr& right)
      : Base(left->position())
      , m_left(left)
      , m_op(op)
      , m_right(right) {}

    inline Kind kind() const override
    {
      return Kind::Binary;
    }

    inline const ptr& left() const
    {
      return m_left;
    }

    inline Token::Kind op() const
    {
      return m_op;
    }

    inline const ptr& right() const
    {
      return m_right;
    }

    static std::u32string ToString(Token::Kind op);

    std::u32string ToString() const override;

  private:
    const ptr m_left;
    const Token::Kind m_op;
    const ptr m_right;
  };

  class Boolean final : public Base
  {
  public:
    explicit Boolean(const Position& position, bool value)
      : Base(position)
      , m_value(value) {}

    inline Kind kind() const override
    {
      return Kind::Boolean;
    }

    inline bool value() const
    {
      return m_value;
    }

    inline std::u32string ToString() const override
    {
      return m_value ? U"true" : U"false";
    }

  private:
    const bool m_value;
  };

  class Call final : public Base
  {
  public:
    explicit Call(
      const Position& position,
      const ptr& expression,
      const std::vector<ptr>& arguments,
      bool conditional
    )
      : Base(position)
      , m_expression(expression)
      , m_arguments(arguments)
      , m_conditional(conditional) {}

    inline Kind kind() const override
    {
      return Kind::Call;
    }

    inline const ptr& expression() const
    {
      return m_expression;
    }

    inline const std::vector<ptr>& arguments() const
    {
      return m_arguments;
    }

    inline bool conditional() const
    {
      return m_conditional;
    }

    std::u32string ToString() const override;

  private:
    const ptr m_expression;
    const std::vector<ptr> m_arguments;
    const bool m_conditional;
  };

  class Float final : public Base
  {
  public:
    explicit Float(const Position& position, double value)
      : Base(position)
      , m_value(value) {}

    inline Kind kind() const override
    {
      return Kind::Float;
    }

    inline double value() const
    {
      return m_value;
    }

    std::u32string ToString() const override;

  private:
    const double m_value;
  };

  class Function final : public Base
  {
  public:
    explicit Function(
      const Position& position,
      const std::vector<parameter::ptr>& parameters,
      const type::ptr& return_type,
      const statement::ptr& body
    )
      : Base(position)
      , m_parameters(parameters)
      , m_return_type(return_type)
      , m_body(body) {}

    inline Kind kind() const override
    {
      return Kind::Function;
    }

    inline const std::vector<parameter::ptr>& parameters() const
    {
      return m_parameters;
    }

    inline const type::ptr& return_type() const
    {
      return m_return_type;
    }

    inline const statement::ptr& body() const
    {
      return m_body;
    }

    std::u32string ToString() const override;

  private:
    const std::vector<parameter::ptr> m_parameters;
    const type::ptr m_return_type;
    const statement::ptr m_body;
  };

  class Id final : public Base
  {
  public:
    explicit Id(const Position& position, const std::u32string& identifier)
      : Base(position)
      , m_identifier(identifier) {}

    inline Kind kind() const override
    {
      return Kind::Id;
    }

    inline const std::u32string& identifier() const
    {
      return m_identifier;
    }

    inline bool IsAssignable() const override
    {
      return true;
    }

    inline std::u32string ToString() const override
    {
      return m_identifier;
    }

  private:
    const std::u32string m_identifier;
  };

  class Int final : public Base
  {
  public:
    explicit Int(const Position& position, std::int64_t value)
      : Base(position)
      , m_value(value) {}

    inline Kind kind() const override
    {
      return Kind::Int;
    }

    inline std::int64_t value() const
    {
      return m_value;
    }

    std::u32string ToString() const override;

  private:
    const std::int64_t m_value;
  };

  class List final : public Base
  {
  public:
    using container_type = std::vector<element::ptr>;

    explicit List(const Position& position, const container_type& elements)
      : Base(position)
      , m_elements(elements) {}

    inline Kind kind() const override
    {
      return Kind::List;
    }

    inline const container_type& elements() const
    {
      return m_elements;
    }

    std::u32string ToString() const override;

  private:
    const container_type m_elements;
  };

  class Null final : public Base
  {
  public:
    explicit Null(const Position& position)
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
    explicit Property(
      const Position& position,
      const ptr& expression,
      const std::u32string& name,
      bool conditional
    )
      : Base(position)
      , m_expression(expression)
      , m_name(name)
      , m_conditional(conditional) {}

    inline Kind kind() const override
    {
      return Kind::Property;
    }

    inline const ptr& expression() const
    {
      return m_expression;
    }

    inline const std::u32string& name() const
    {
      return m_name;
    }

    inline bool conditional() const
    {
      return m_conditional;
    }

    std::u32string ToString() const override;

  private:
    const ptr m_expression;
    const std::u32string m_name;
    const bool m_conditional;
  };

  class Record final : public Base
  {
  public:
    using container_type = std::vector<field::ptr>;

    explicit Record(const Position& position, const container_type& fields)
      : Base(position)
      , m_fields(fields) {}

    inline Kind kind() const override
    {
      return Kind::Record;
    }

    inline const container_type& fields() const
    {
      return m_fields;
    }

    std::u32string ToString() const override;

  private:
    const container_type m_fields;
  };

  class String final : public Base
  {
  public:
    explicit String(const Position& position, const std::u32string& value)
      : Base(position)
      , m_value(value) {}

    inline Kind kind() const override
    {
      return Kind::String;
    }

    inline const std::u32string& value() const
    {
      return m_value;
    }

    std::u32string ToString() const override;

  private:
    const std::u32string m_value;
  };

  class Ternary final : public Base
  {
  public:
    explicit Ternary(
      const Position& position,
      const ptr& condition,
      const ptr& then_expression,
      const ptr& else_expression
    )
      : Base(position)
      , m_condition(condition)
      , m_then_expression(then_expression)
      , m_else_expression(else_expression) {}

    inline Kind kind() const override
    {
      return Kind::Ternary;
    }

    inline const ptr& condition() const
    {
      return m_condition;
    }

    inline const ptr& then_expression() const
    {
      return m_then_expression;
    }

    inline const ptr& else_expression() const
    {
      return m_else_expression;
    }

    std::u32string ToString() const override;

  private:
    const ptr m_condition;
    const ptr m_then_expression;
    const ptr m_else_expression;
  };

  class Subscript final : public Base
  {
  public:
    explicit Subscript(
      const Position& position,
      const ptr& expression,
      const ptr& index,
      bool conditional
    )
      : Base(position)
      , m_expression(expression)
      , m_index(index)
      , m_conditional(conditional) {}

    inline Kind kind() const override
    {
      return Kind::Subscript;
    }

    inline const ptr& expression() const
    {
      return m_expression;
    }

    inline const ptr& index() const
    {
      return m_index;
    }

    inline bool conditional() const
    {
      return m_conditional;
    }

    std::u32string ToString() const override;

  private:
    const ptr m_expression;
    const ptr m_index;
    const bool m_conditional;
  };

  class Unary final : public Base
  {
  public:
    explicit Unary(const Position& position, Token::Kind op, const ptr& operand)
      : Base(position)
      , m_op(op)
      , m_operand(operand) {}

    inline Kind kind() const override
    {
      return Kind::Unary;
    }

    inline Token::Kind op() const
    {
      return m_op;
    }

    inline const ptr& operand() const
    {
      return m_operand;
    }

    static std::u32string ToString(Token::Kind op);

    std::u32string ToString() const override;

  private:
    const Token::Kind m_op;
    const ptr m_operand;
  };
}
