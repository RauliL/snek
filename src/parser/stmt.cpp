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
#include <snek/ast/stmt/assign.hpp>
#include <snek/ast/stmt/block.hpp>
#include <snek/ast/stmt/break.hpp>
#include <snek/ast/stmt/continue.hpp>
#include <snek/ast/stmt/expr.hpp>
#include <snek/ast/stmt/if.hpp>
#include <snek/ast/stmt/import.hpp>
#include <snek/ast/stmt/return.hpp>
#include <snek/ast/stmt/type.hpp>
#include <snek/ast/stmt/while.hpp>
#include <snek/parser.hpp>

namespace snek::parser::stmt
{
  static std::optional<Error>
  skip_new_line(State& state)
  {
    if (!state.eof())
    {
      const auto& token = *state.current++;

      if (token.kind() != cst::Kind::NewLine)
      {
        return std::make_optional<Error>({
          token.position(),
          U"Unexpected " +
          cst::to_string(token.kind())
          + U"; Missing new line."
        });
      }
    }

    return std::nullopt;
  }

  static result_type
  parse_import_stmt(State& state)
  {
    const auto position = state.current++->position();
    ast::stmt::Import::container_type specifiers;
    std::u32string path;

    do
    {
      const auto specifier = import::parse(state, position);

      if (!specifier)
      {
        return result_type::error(specifier.error());
      }
      specifiers.push_back(specifier.value());
    }
    while (state.peek_read(cst::Kind::Comma));

    if (!state.peek_read(cst::Kind::KeywordFrom) ||
        !state.peek(cst::Kind::Str))
    {
      return result_type::error({
        position,
        U"Missing module path in `import' statement."
      });
    }
    path = *state.current++->text();
    if (const auto error = skip_new_line(state))
    {
      return result_type::error(*error);
    }

    return result_type::ok(std::make_shared<ast::stmt::Import>(
      position,
      path,
      specifiers
    ));
  }

  result_type
  parse_block(State& state, const ast::Position& position)
  {
    if (state.peek_read(cst::Kind::NewLine))
    {
      ast::stmt::Block::container_type statements;

      if (!state.peek_read(cst::Kind::Indent))
      {
        return result_type::error({
          position,
          U"Missing block."
        });
      }
      for (;;)
      {
        const auto statement = parse(state);

        if (!statement)
        {
          return statement;
        }
        statements.push_back(statement.value());
        if (state.eof() || state.peek_read(cst::Kind::Dedent))
        {
          break;
        }
      }

      return result_type::ok(std::make_shared<ast::stmt::Block>(
        position,
        statements
      ));
    }

    return parse(state);
  }

  static result_type
  parse_if_stmt(State& state)
  {
    const auto position = state.current++->position();
    const auto condition = expr::parse(state);
    std::shared_ptr<ast::stmt::Base> then_statement;
    std::shared_ptr<ast::stmt::Base> else_statement;

    if (!condition)
    {
      return result_type::error(condition.error());
    }
    else if (!state.peek_read(cst::Kind::Colon))
    {
      return result_type::error({
        position,
        U"Missing `:' after `if' statement."
      });
    }

    const auto then_block = parse_block(state, position);

    if (!then_block)
    {
      return then_block;
    }
    then_statement = then_block.value();

    if (state.peek_read(cst::Kind::KeywordElse))
    {
      if (state.peek(cst::Kind::KeywordIf))
      {
        const auto else_if_stmt = parse_if_stmt(state);

        if (!else_if_stmt)
        {
          return else_if_stmt;
        }
        else_statement = else_if_stmt.value();
      }
      else if (state.peek_read(cst::Kind::Colon))
      {
        const auto else_block = parse_block(state, position);

        if (!else_block)
        {
          return else_block;
        }
        else_statement = else_block.value();
      } else {
        return result_type::error({
          position,
          U"Missing `:' after `else' statement."
        });
      }
    }

    return result_type::ok(std::make_shared<ast::stmt::If>(
      position,
      condition.value(),
      then_statement,
      else_statement
    ));
  }

  static result_type
  parse_while_stmt(State& state)
  {
    const auto position = state.current++->position();
    const auto condition = expr::parse(state);

    if (!condition)
    {
      return result_type::error(condition.error());
    }
    else if (!state.peek_read(cst::Kind::Colon))
    {
      return result_type::error({
        position,
        U"Missing `:' after `while' statement."
      });
    }

    const auto statement = parse_block(state, position);

    if (!statement)
    {
      return statement;
    }

    return result_type::ok(std::make_shared<ast::stmt::While>(
      position,
      condition.value(),
      statement.value()
    ));
  }

  static result_type
  parse_return_stmt(State& state)
  {
    const auto position = state.current++->position();
    std::shared_ptr<ast::expr::RValue> value_expression;

    if (!state.eof() &&
        !state.peek(cst::Kind::NewLine) &&
        !state.peek(cst::Kind::Dedent) &&
        !state.peek(cst::Kind::Semicolon))
    {
      const auto result = expr::parse(state);

      if (!result)
      {
        return result_type::error(result.error());
      }
      else if (const auto error = skip_new_line(state))
      {
        return result_type::error(*error);
      }
      value_expression = result.value();
    }

    return result_type::ok(std::make_shared<ast::stmt::Return>(
      position,
      value_expression
    ));
  }

  static result_type
  parse_break_stmt(State& state)
  {
    const auto position = state.current++->position();

    if (const auto error = skip_new_line(state))
    {
      return result_type::error(*error);
    }

    return result_type::ok(std::make_shared<ast::stmt::Break>(position));
  }

  static result_type
  parse_continue_stmt(State& state)
  {
    const auto position = state.current++->position();

    if (const auto error = skip_new_line(state))
    {
      return result_type::error(*error);
    }

    return result_type::ok(std::make_shared<ast::stmt::Continue>(position));
  }

  static result_type
  parse_type_stmt(State& state)
  {
    const auto position = state.current++->position();
    std::u32string name;

    if (!state.peek(cst::Kind::Id))
    {
      return result_type::error({
        position,
        U"Missing identifier after `type'."
      });
    }
    name = *state.current++->text();
    if (!state.peek_read(cst::Kind::Assign))
    {
      return result_type::error({
        position,
        U"Missing `=' after `type'."
      });
    }

    const auto type = type::parse(state, position);

    if (!type)
    {
      return result_type::error(type.error());
    }
    else if (const auto error = skip_new_line(state))
    {
      return result_type::error(*error);
    }

    return result_type::ok(std::make_shared<ast::stmt::Type>(
      position,
      name,
      type.value()
    ));
  }

  static result_type
  parse_assign(State& state, const std::shared_ptr<ast::expr::RValue>& target)
  {
    const auto position = state.current++->position();
    std::shared_ptr<ast::expr::RValue> value;

    if (target->kind() != ast::expr::Kind::LValue)
    {
      return result_type::error({ position, U"Cannot assign into TODO." });
    }
    if (state.peek_func())
    {
      const auto result = expr::parse_func(state);

      if (!result)
      {
        return result_type::error(result.error());
      }
      value = result.value();
    } else {
      const auto result = expr::parse(state);

      if (!result)
      {
        return result_type::error(result.error());
      }
      else if (const auto error = skip_new_line(state))
      {
        return result_type::error(*error);
      }
      value = result.value();
    }

    return result_type::ok(std::make_shared<ast::stmt::Assign>(
      position,
      std::static_pointer_cast<ast::expr::LValue>(target),
      value
    ));
  }

  static result_type
  parse_expr_stmt(State& state)
  {
    const auto expr = expr::parse(state);

    if (!expr)
    {
      return result_type::error(expr.error());
    }
    else if (state.peek(cst::Kind::Assign))
    {
      return parse_assign(state, expr.value());
    }
    else if (const auto error = skip_new_line(state))
    {
      return result_type::error(*error);
    }

    return result_type::ok(std::make_shared<ast::stmt::Expr>(
      expr.value()->position(),
      expr.value()
    ));
  }

  result_type
  parse(State& state)
  {
    if (state.eof())
    {
      return result_type::error({
        std::nullopt,
        U"Unexpected end of input; Missing statement."
      });
    }
    switch (state.current->kind())
    {
      case cst::Kind::KeywordImport:
        return parse_import_stmt(state);

      case cst::Kind::KeywordIf:
        return parse_if_stmt(state);

      case cst::Kind::KeywordWhile:
        return parse_while_stmt(state);

      case cst::Kind::KeywordReturn:
        return parse_return_stmt(state);

      case cst::Kind::KeywordBreak:
        return parse_break_stmt(state);

      case cst::Kind::KeywordContinue:
        return parse_continue_stmt(state);

      case cst::Kind::KeywordType:
        return parse_type_stmt(state);

      default:
        return parse_expr_stmt(state);
    }
  }
}
