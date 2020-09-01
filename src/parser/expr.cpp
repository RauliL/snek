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
#include <snek/ast/expr/binary.hpp>
#include <snek/ast/expr/bool.hpp>
#include <snek/ast/expr/call.hpp>
#include <snek/ast/expr/field.hpp>
#include <snek/ast/expr/float.hpp>
#include <snek/ast/expr/func.hpp>
#include <snek/ast/expr/id.hpp>
#include <snek/ast/expr/int.hpp>
#include <snek/ast/expr/list.hpp>
#include <snek/ast/expr/not.hpp>
#include <snek/ast/expr/null.hpp>
#include <snek/ast/expr/record.hpp>
#include <snek/ast/expr/str.hpp>
#include <snek/ast/parameter.hpp>
#include <snek/ast/record.hpp>
#include <snek/ast/stmt/block.hpp>
#include <snek/parser.hpp>

namespace snek::parser::expr
{
  using expr_list_result_type = peelo::result<
    std::vector<std::shared_ptr<ast::expr::RValue>>,
    Error
  >;

  static expr_list_result_type
  parse_expr_list(
    State& state,
    const ast::Position& position,
    cst::Kind separator,
    const std::u32string& what
  )
  {
    std::vector<std::shared_ptr<ast::expr::RValue>> list;

    for (;;)
    {
      if (state.eof())
      {
        return expr_list_result_type::error({
          position,
          U"Unterminated " +
          what +
          U"; Missing " +
          cst::to_string(separator)
          + U"."
        });
      }
      else if (state.peek_read(separator))
      {
        break;
      } else {
        const auto expr = parse(state);

        if (expr)
        {
          list.push_back(expr.value());
          if (!state.peek(cst::Kind::Comma) && !state.peek(separator))
          {
            return expr_list_result_type::error({
              expr.value()->position(),
              U"Unterminated " +
              what + +
              U"; Missing " +
              cst::to_string(separator) +
              U"."
            });
          }
          state.peek_read(cst::Kind::Comma);
        } else {
          return expr_list_result_type::error(expr.error());
        }
      }
    }

    return expr_list_result_type::ok(list);
  }

  static result_type
  parse_list_expr(State& state)
  {
    const auto position = state.current++->position();
    const auto elements = parse_expr_list(
      state,
      position,
      cst::Kind::RightBracket,
      U"list literal"
    );

    if (!elements)
    {
      return result_type::error(elements.error());
    }

    return result_type::ok(std::make_shared<ast::expr::List>(
      position,
      elements.value()
    ));
  }

  static result_type
  parse_record_expr(State& state)
  {
    const auto position = state.current++->position();
    ast::expr::Record::container_type fields;

    for (;;)
    {
      if (state.eof())
      {
        return result_type::error({
          position,
          U"Unterminated record; Missing `}'."
        });
      }
      else if (state.peek_read(cst::Kind::RightBrace))
      {
        break;
      } else {
        const auto field = record::parse(state, position);

        if (!field)
        {
          return result_type::error(field.error());
        }
        fields.push_back(field.value());
        if (!state.peek(cst::Kind::Comma) &&
            !state.peek(cst::Kind::RightBrace))
        {
          return result_type::error({
            field.value()->position(),
            U"Unterminated record; Missing `}'."
          });
        }
        state.peek_read(cst::Kind::Comma);
      }
    }

    return result_type::ok(std::make_shared<ast::expr::Record>(
      position,
      fields
    ));
  }

  result_type
  parse_func(State& state)
  {
    const auto position = state.current->position();
    const auto parameters = parse_parameter_list(state);
    std::optional<std::shared_ptr<ast::type::Base>> return_type;

    if (!parameters)
    {
      return result_type::error(parameters.error());
    }
    if (state.peek_read(cst::Kind::Arrow))
    {
      const auto result = type::parse(state, position);

      if (!result)
      {
        return result_type::error(result.error());
      }
      return_type = result.value();
    }
    if (!state.peek_read(cst::Kind::Colon))
    {
      return result_type::error({
        position,
        U"Missing `:' after function declaration."
      });
    }

    const auto body = stmt::parse_block(state, position);

    if (!body)
    {
      return result_type::error(body.error());
    }

    return result_type::ok(std::make_shared<ast::expr::Func>(
      position,
      parameters.value(),
      std::static_pointer_cast<ast::stmt::Block>(body.value()),
      return_type
    ));
  }

  static result_type
  parse_parenthesized_expr(State& state)
  {
    const auto position = state.current++->position();
    const auto expression = parse(state);

    if (!expression)
    {
      return expression;
    }
    else if (!state.peek_read(cst::Kind::RightParen))
    {
      return result_type::error({
        position,
        U"Unterminated parenthesized expression; Missing `)'."
      });
    }

    return expression;
  }

  static result_type
  parse_primary_expr(State& state)
  {
    std::shared_ptr<ast::expr::RValue> expr;

    if (state.eof())
    {
      return result_type::error({
        std::nullopt,
        U"Unexpected end of input; Missing expression."
      });
    }
    switch (state.current->kind())
    {
      case cst::Kind::Float:
        // TODO: Deal with thrown exceptions.
        expr = std::make_shared<ast::expr::Float>(
          state.current->position(),
          std::strtod(
            peelo::unicode::encoding::utf8::encode(
              *state.current->text()
            ).c_str(),
            nullptr
          )
        );
        break;

      case cst::Kind::Int:
        // TODO: Deal with thrown exceptions.
        expr = std::make_shared<ast::expr::Int>(
          state.current->position(),
          std::strtoll(
            peelo::unicode::encoding::utf8::encode(
              *state.current->text()
            ).c_str(),
            nullptr,
            10
          )
        );
        break;

      case cst::Kind::Str:
        expr = std::make_shared<ast::expr::Str>(
          state.current->position(),
          *state.current->text()
        );
        break;

      case cst::Kind::Id:
        expr = std::make_shared<ast::expr::Id>(
          state.current->position(),
          *state.current->text()
        );
        break;

      case cst::Kind::LeftBracket:
        return parse_list_expr(state);

      case cst::Kind::LeftBrace:
        return parse_record_expr(state);

      case cst::Kind::LeftParen:
        return state.peek_func()
          ? parse_func(state)
          : parse_parenthesized_expr(state);

      case cst::Kind::KeywordNull:
        expr = std::make_shared<ast::expr::Null>(state.current->position());
        break;

      case cst::Kind::KeywordFalse:
        expr = std::make_shared<ast::expr::Bool>(
          state.current->position(),
          false
        );
        break;

      case cst::Kind::KeywordTrue:
        expr = std::make_shared<ast::expr::Bool>(
          state.current->position(),
          true
        );
        break;

      default:
        return result_type::error({
          state.current->position(),
          U"Unexpected " +
          cst::to_string(state.current->kind())
          + U"; Missing expression."
        });
    }
    ++state.current;

    return result_type::ok(expr);
  }

  static result_type
  parse_call_expr(
    State& state,
    const ast::Position& position,
    const std::shared_ptr<ast::expr::RValue>& callee
  )
  {
    const auto arguments = parse_expr_list(
      state,
      position,
      cst::Kind::RightParen,
      U"argument list"
    );

    if (!arguments)
    {
      return result_type::error(arguments.error());
    }

    return result_type::ok(std::make_shared<ast::expr::Call>(
      position,
      callee,
      arguments.value()
    ));
  }

  static result_type
  parse_field_expr(
    State& state,
    const ast::Position& position,
    const std::shared_ptr<ast::expr::RValue>& record
  )
  {
    if (state.eof() || state.current->kind() != cst::Kind::Id)
    {
      return result_type::error({
        position,
        U"Missing identifier after `.'."
      });
    }

    return result_type::ok(std::make_shared<ast::expr::Field>(
      position,
      record,
      *(state.current++)->text()
    ));
  }

  static result_type
  parse_selector(
    State& state,
    const std::shared_ptr<ast::expr::RValue>& target
  )
  {
    const auto& selector = *state.current++;

    if (selector.kind() == cst::Kind::LeftParen)
    {
      return parse_call_expr(state, selector.position(), target);
    } else {
      return parse_field_expr(state, selector.position(), target);
    }
  }

  static result_type
  parse_unary_expr(State& state)
  {
    if (state.peek(cst::Kind::Not))
    {
      const auto position = state.current++->position();
      const auto expression = parse_unary_expr(state);

      if (!expression)
      {
        return expression;
      }

      return result_type::ok(std::make_shared<ast::expr::Not>(
        position,
        expression.value()
      ));
    }

    auto expression = parse_primary_expr(state);

    if (!expression)
    {
      return expression;
    }

    while (state.peek(cst::Kind::Dot) || state.peek(cst::Kind::LeftParen))
    {
      expression = parse_selector(state, expression.value());
      if (!expression)
      {
        return expression;
      }
    }

    return expression;
  }

  static result_type
  parse_multiplicative_expr(State& state)
  {
    auto expression = parse_unary_expr(state);

    if (!expression)
    {
      return expression;
    }
    while (state.peek(cst::Kind::Mul) ||
           state.peek(cst::Kind::Div) ||
           state.peek(cst::Kind::Mod))
    {
      const auto position = state.current->position();
      const auto kind = state.current++->kind();
      const auto operand = parse_unary_expr(state);

      if (!operand)
      {
        return operand;
      }
      expression = result_type::ok(std::make_shared<ast::expr::Binary>(
        position,
        expression.value(),
        kind == cst::Kind::Mul
          ? ast::expr::BinaryOperator::Mul
          : kind == cst::Kind::Div
          ? ast::expr::BinaryOperator::Div
          : ast::expr::BinaryOperator::Mod,
        operand.value()
      ));
    }

    return expression;
  }

  static result_type
  parse_additive_expression(State& state)
  {
    auto expression = parse_multiplicative_expr(state);

    if (!expression)
    {
      return expression;
    }
    while (state.peek(cst::Kind::Add) || state.peek(cst::Kind::Sub))
    {
      const auto position = state.current->position();
      const auto kind = state.current++->kind();
      const auto operand = parse_multiplicative_expr(state);

      if (!operand)
      {
        return operand;
      }
      expression = result_type::ok(std::make_shared<ast::expr::Binary>(
        position,
        expression.value(),
        kind == cst::Kind::Add
          ? ast::expr::BinaryOperator::Add
          : ast::expr::BinaryOperator::Sub,
        operand.value()
      ));
    }

    return expression;
  }

  static result_type
  parse_relational_expr(State& state)
  {
    auto expression = parse_additive_expression(state);

    if (!expression)
    {
      return expression;
    }
    while (state.peek(cst::Kind::Lt) ||
           state.peek(cst::Kind::Gt) ||
           state.peek(cst::Kind::Lte) ||
           state.peek(cst::Kind::Gte))
    {
      const auto position = state.current->position();
      const auto kind = state.current++->kind();
      const auto operand = parse_additive_expression(state);

      if (!operand)
      {
        return operand;
      }
      expression = result_type::ok(std::make_shared<ast::expr::Binary>(
        position,
        expression.value(),
        kind == cst::Kind::Lt
          ? ast::expr::BinaryOperator::Lt
          : kind == cst::Kind::Gt
          ? ast::expr::BinaryOperator::Gt
          : kind == cst::Kind::Lte
          ? ast::expr::BinaryOperator::Lte
          : ast::expr::BinaryOperator::Gte,
        operand.value()
      ));
    }

    return expression;
  }

  static result_type
  parse_equality_expr(State& state)
  {
    auto expression = parse_relational_expr(state);

    if (!expression)
    {
      return expression;
    }
    while (state.peek(cst::Kind::Eq) || state.peek(cst::Kind::Ne))
    {
      const auto position = state.current->position();
      const auto kind = state.current++->kind();
      const auto operand = parse_relational_expr(state);

      if (!operand)
      {
        return operand;
      }
      expression = result_type::ok(std::make_shared<ast::expr::Binary>(
        position,
        expression.value(),
        kind == cst::Kind::Eq
          ? ast::expr::BinaryOperator::Eq
          : ast::expr::BinaryOperator::Ne,
        operand.value()
      ));
    }

    return expression;
  }

  result_type
  parse(State& state)
  {
    return parse_equality_expr(state);
  }
}
