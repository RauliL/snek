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

    explicit Base(const std::optional<Position>& position)
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

    const container_type statements;

    explicit Block(
      const std::optional<Position>& position,
      const container_type& statements_
    )
      : Base(position)
      , statements(statements_) {}

    inline Kind kind() const override
    {
      return Kind::Block;
    }

    inline std::u32string ToString() const override
    {
      return U"...";
    }
  };

  class DeclareType final : public Base
  {
  public:
    const bool is_export;
    const std::u32string name;
    const type::ptr type;

    explicit DeclareType(
      const std::optional<Position>& position,
      bool is_export_,
      const std::u32string name_,
      const type::ptr type_
    )
      : Base(position)
      , is_export(is_export_)
      , name(name_)
      , type(type_) {}

    inline Kind kind() const override
    {
      return Kind::DeclareType;
    }

    std::u32string ToString() const override;
  };

  class DeclareVar final : public Base
  {
  public:
    const bool is_export;
    const bool is_read_only;
    const expression::ptr variable;
    const expression::ptr value;

    explicit DeclareVar(
      const std::optional<Position>& position,
      bool is_export_,
      bool is_read_only_,
      const expression::ptr& variable_,
      const expression::ptr& value_
    )
      : Base(position)
      , is_export(is_export_)
      , is_read_only(is_read_only_)
      , variable(variable_)
      , value(value_) {}

    inline Kind kind() const override
    {
      return Kind::DeclareVar;
    }

    std::u32string ToString() const override;
  };

  class Expression final : public Base
  {
  public:
    const expression::ptr expression;

    explicit Expression(const expression::ptr& expression_)
      : Base(expression_->position)
      , expression(expression_) {}

    inline Kind kind() const override
    {
      return Kind::Expression;
    }

    inline std::u32string ToString() const override
    {
      return expression->ToString();
    }
  };

  class If final : public Base
  {
  public:
    const expression::ptr condition;
    const ptr then_statement;
    const ptr else_statement;

    explicit If(
      const std::optional<Position>& position,
      const expression::ptr& condition_,
      const ptr& then_statement_,
      const ptr& else_statement_ = nullptr
    )
      : Base(position)
      , condition(condition_)
      , then_statement(then_statement_)
      , else_statement(else_statement_) {}

    inline Kind kind() const override
    {
      return Kind::If;
    }

    std::u32string ToString() const override;
  };

  class Import final : public Base
  {
  public:
    using container_type = std::vector<import::ptr>;

    const container_type specifiers;
    const std::u32string path;

    explicit Import(
      const std::optional<Position>& position,
      const container_type& specifiers_,
      const std::u32string& path_
    )
      : Base(position)
      , specifiers(specifiers_)
      , path(path_) {}

    inline Kind kind() const override
    {
      return Kind::Import;
    }

    std::u32string ToString() const override;
  };

  class Jump final : public Base
  {
  public:
    const JumpKind jump_kind;
    const expression::ptr value;

    explicit Jump(
      const std::optional<Position>& position,
      JumpKind jump_kind_,
      const expression::ptr& value_ = nullptr
    )
      : Base(position)
      , jump_kind(jump_kind_)
      , value(value_) {}

    inline Kind kind() const override
    {
      return Kind::Jump;
    }

    static std::u32string ToString(JumpKind kind);

    std::u32string ToString() const override;
  };

  class While final : public Base
  {
  public:
    const expression::ptr condition;
    const ptr body;

    explicit While(
      const std::optional<Position>& position,
      const expression::ptr& condition_,
      const ptr& body_
    )
      : Base(position)
      , condition(condition_)
      , body(body_) {}

    inline Kind kind() const override
    {
      return Kind::While;
    }

    std::u32string ToString() const override;
  };
}
