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

namespace snek::parser
{
  enum class StatementKind
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
    Break = static_cast<int>(TokenKind::KeywordBreak),
    Continue = static_cast<int>(TokenKind::KeywordContinue),
    Return = static_cast<int>(TokenKind::KeywordReturn),
  };

  class Statement : public TypedNode<StatementKind>
  {
  public:
    DISALLOW_COPY_AND_ASSIGN(Statement);

    class Block;
    class DeclareType;
    class DeclareVar;
    class Expression;
    class If;
    class Import;
    class Jump;
    class While;

    explicit Statement(
      const std::optional<Position>& position,
      StatementKind kind
    )
      : TypedNode(position, kind) {}

    static statement_ptr Parse(Lexer& lexer, bool is_top_level);

    static statement_ptr ParseBlock(Lexer& lexer);

    static statement_ptr ParseFunctionBody(Lexer& lexer);
  };

  class Statement::Block final : public Statement
  {
  public:
    using value_type = statement_ptr;
    using container_type = std::vector<value_type>;

    const container_type statements;

    explicit Block(
      const std::optional<Position>& position,
      const container_type& statements_
    )
      : Statement(position, StatementKind::Block)
      , statements(statements_) {}

    inline std::u32string ToString() const override
    {
      return U"...";
    }
  };

  class Statement::DeclareType final : public Statement
  {
  public:
    const bool is_export;
    const std::u32string name;
    const type_ptr type;

    explicit DeclareType(
      const std::optional<Position>& position,
      bool is_export_,
      const std::u32string name_,
      const type_ptr type_
    )
      : Statement(position, StatementKind::DeclareType)
      , is_export(is_export_)
      , name(name_)
      , type(type_) {}

    std::u32string ToString() const override;
  };

  class Statement::DeclareVar final : public Statement
  {
  public:
    const bool is_export;
    const bool is_read_only;
    const expression_ptr variable;
    const expression_ptr value;

    explicit DeclareVar(
      const std::optional<Position>& position,
      bool is_export_,
      bool is_read_only_,
      const expression_ptr& variable_,
      const expression_ptr& value_
    )
      : Statement(position, StatementKind::DeclareVar)
      , is_export(is_export_)
      , is_read_only(is_read_only_)
      , variable(variable_)
      , value(value_) {}

    std::u32string ToString() const override;
  };

  class Statement::Expression final : public Statement
  {
  public:
    const expression_ptr expression;

    explicit Expression(const expression_ptr& expression_)
      : Statement(expression_->position, StatementKind::Expression)
      , expression(expression_) {}

    inline std::u32string ToString() const override
    {
      return expression->ToString();
    }
  };

  class Statement::If final : public Statement
  {
  public:
    const expression_ptr condition;
    const statement_ptr then_statement;
    const statement_ptr else_statement;

    explicit If(
      const std::optional<Position>& position,
      const expression_ptr& condition_,
      const statement_ptr& then_statement_,
      const statement_ptr& else_statement_ = nullptr
    )
      : Statement(position, StatementKind::If)
      , condition(condition_)
      , then_statement(then_statement_)
      , else_statement(else_statement_) {}

    std::u32string ToString() const override;
  };

  class Statement::Import final : public Statement
  {
  public:
    using value_type = import_ptr;
    using container_type = std::vector<import_ptr>;

    const container_type specifiers;
    const std::u32string path;

    explicit Import(
      const std::optional<Position>& position,
      const container_type& specifiers_,
      const std::u32string& path_
    )
      : Statement(position, StatementKind::Import)
      , specifiers(specifiers_)
      , path(path_) {}

    std::u32string ToString() const override;
  };

  class Statement::Jump final : public Statement
  {
  public:
    const JumpKind jump_kind;
    const expression_ptr value;

    explicit Jump(
      const std::optional<Position>& position,
      JumpKind jump_kind_,
      const expression_ptr& value_ = nullptr
    )
      : Statement(position, StatementKind::Jump)
      , jump_kind(jump_kind_)
      , value(value_) {}

    inline bool Is(JumpKind jump_kind_) const
    {
      return jump_kind == jump_kind_;
    }

    static std::u32string ToString(JumpKind kind);

    std::u32string ToString() const override;
  };

  class Statement::While final : public Statement
  {
  public:
    const expression_ptr condition;
    const statement_ptr body;

    explicit While(
      const std::optional<Position>& position,
      const expression_ptr& condition_,
      const statement_ptr& body_
    )
      : Statement(position, StatementKind::While)
      , condition(condition_)
      , body(body_) {}

    std::u32string ToString() const override;
  };
}
