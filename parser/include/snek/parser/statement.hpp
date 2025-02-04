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

#include "snek/parser/expression.hpp"

namespace snek::parser::import
{
  class Base;

  using ptr = std::shared_ptr<Base>;
}

namespace snek::parser::statement
{
  enum class Kind
  {
    Block,
    DeclareType,
    DeclareVar,
    Expression,
    If,
    Import,
    Jump,
    While,
  };

  class Jump;

  enum class JumpKind
  {
    Break = static_cast<int>(Token::Kind::KeywordBreak),
    Continue = static_cast<int>(Token::Kind::KeywordContinue),
    Return = static_cast<int>(Token::Kind::KeywordReturn),
  };

  class Base : public Node
  {
  public:
    DISALLOW_COPY_AND_ASSIGN(Base);

    explicit Base(const Position& position)
      : Node(position) {}

    virtual Kind kind() const = 0;
  };

  using ptr = std::shared_ptr<Base>;

  ptr Parse(Lexer& lexer, bool is_top_level);

  ptr ParseBlock(Lexer& lexer);

  ptr ParseFunctionBody(Lexer& lexer);

  class Block final : public Base
  {
  public:
    using container_type = std::vector<ptr>;

    explicit Block(
      const Position& position,
      const container_type& statements
    )
      : Base(position)
      , m_statements(statements) {}

    inline Kind kind() const override
    {
      return Kind::Block;
    }

    inline const container_type& statements() const
    {
      return m_statements;
    }

    inline std::u32string ToString() const override
    {
      return U"...";
    }

  private:
    const container_type m_statements;
  };

  class DeclareType final : public Base
  {
  public:
    explicit DeclareType(
      const Position& position,
      bool is_export,
      const std::u32string& name,
      const type::ptr& type
    )
      : Base(position)
      , m_exported(is_export)
      , m_name(name)
      , m_type(type) {}

    inline Kind kind() const override
    {
      return Kind::DeclareType;
    }

    inline bool exported() const
    {
      return m_exported;
    }

    inline const std::u32string& name() const
    {
      return m_name;
    }

    inline const type::ptr& type() const
    {
      return m_type;
    }

    std::u32string ToString() const override;

  private:
    const bool m_exported;
    const std::u32string m_name;
    const type::ptr m_type;
  };

  class DeclareVar final : public Base
  {
  public:
    explicit DeclareVar(
      const Position& position,
      bool exported,
      bool read_only,
      const std::u32string& name,
      const expression::ptr& value
    )
      : Base(position)
      , m_exported(exported)
      , m_read_only(read_only)
      , m_name(name)
      , m_value(value) {}

    inline Kind kind() const override
    {
      return Kind::DeclareVar;
    }

    inline bool exported() const
    {
      return m_exported;
    }

    inline bool read_only() const
    {
      return m_read_only;
    }

    inline const std::u32string& name() const
    {
      return m_name;
    }

    inline const expression::ptr& value() const
    {
      return m_value;
    }

    std::u32string ToString() const override;

  private:
    const bool m_exported;
    const bool m_read_only;
    const std::u32string m_name;
    const expression::ptr m_value;
  };

  class Expression final : public Base
  {
  public:
    explicit Expression(const expression::ptr& expression)
      : Base(expression->position())
      , m_expression(expression) {}

    inline Kind kind() const override
    {
      return Kind::Expression;
    }

    inline const expression::ptr& expression() const
    {
      return m_expression;
    }

    inline std::u32string ToString() const override
    {
      return m_expression->ToString().append(1, U';');
    }

  private:
    const expression::ptr m_expression;
  };

  class If final : public Base
  {
  public:
    explicit If(
      const Position& position,
      const expression::ptr& condition,
      const ptr& then_statement,
      const ptr& else_statement = nullptr
    )
      : Base(position)
      , m_condition(condition)
      , m_then_statement(then_statement)
      , m_else_statement(else_statement) {}

    inline Kind kind() const override
    {
      return Kind::If;
    }

    inline const expression::ptr& condition() const
    {
      return m_condition;
    }

    inline const ptr& then_statement() const
    {
      return m_then_statement;
    }

    inline const ptr& else_statement() const
    {
      return m_else_statement;
    }

    std::u32string ToString() const override;

  private:
    const expression::ptr m_condition;
    const ptr m_then_statement;
    const ptr m_else_statement;
  };

  class Import final : public Base
  {
  public:
    using container_type = std::vector<import::ptr>;

    explicit Import(
      const Position& position,
      const container_type& specifiers,
      const std::u32string& path
    )
      : Base(position)
      , m_specifiers(specifiers)
      , m_path(path) {}

    inline Kind kind() const override
    {
      return Kind::Import;
    }

    inline const container_type& specifiers() const
    {
      return m_specifiers;
    }

    inline const std::u32string& path() const
    {
      return m_path;
    }

    std::u32string ToString() const override;

  private:
    const container_type m_specifiers;
    const std::u32string m_path;
  };

  class Jump final : public Base
  {
  public:
    explicit Jump(
      const Position& position,
      JumpKind jump_kind,
      const expression::ptr& value = nullptr
    )
      : Base(position)
      , m_jump_kind(jump_kind)
      , m_value(value) {}

    inline Kind kind() const override
    {
      return Kind::Jump;
    }

    inline JumpKind jump_kind() const
    {
      return m_jump_kind;
    }

    inline const expression::ptr& value() const
    {
      return m_value;
    }

    static std::u32string ToString(JumpKind kind);

    std::u32string ToString() const override;

  private:
    const JumpKind m_jump_kind;
    const expression::ptr m_value;
  };

  class While final : public Base
  {
  public:
    explicit While(
      const Position& position,
      const expression::ptr& condition,
      const ptr& body
    )
      : Base(position)
      , m_condition(condition)
      , m_body(body) {}

    inline Kind kind() const override
    {
      return Kind::While;
    }

    inline const expression::ptr& condition() const
    {
      return m_condition;
    }

    inline const ptr& body() const
    {
      return m_body;
    }

    std::u32string ToString() const override;

  private:
    const expression::ptr m_condition;
    const ptr m_body;
  };
}
