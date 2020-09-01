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
#include <snek/ast/record.hpp>
#include <snek/parser.hpp>

namespace snek::parser::record
{
  static result_type
  parse_named(State& state)
  {
    const auto position = state.current->position();
    const auto name = *state.current++->text();

    if (state.peek_read(cst::Kind::Colon))
    {
      const auto value_expression = expr::parse(state);

      if (!value_expression)
      {
        return result_type::error(value_expression.error());
      }

      return result_type::ok(std::make_shared<ast::record::Named>(
        position,
        name,
        value_expression.value()
      ));
    }

    return result_type::ok(std::make_shared<ast::record::Var>(position, name));
  }

  static result_type
  parse_expr(State& state)
  {
    const auto position = state.current++->position();
    const auto name_expression = expr::parse(state);

    if (!name_expression)
    {
      return result_type::error(name_expression.error());
    }
    else if (!state.peek_read(cst::Kind::RightBracket))
    {
      return result_type::error({
        position,
        U"Missing `]'."
      });
    }
    else if (!state.peek_read(cst::Kind::Colon))
    {
      return result_type::error({
        position,
        U"Missing `:'."
      });
    }

    const auto value_expression = expr::parse(state);

    if (!value_expression)
    {
      return result_type::error(value_expression.error());
    }

    return result_type::ok(std::make_shared<ast::record::Expr>(
      position,
      name_expression.value(),
      value_expression.value()
    ));
  }

  static result_type
  parse_spread(State& state)
  {
    const auto position = state.current++->position();
    const auto expression = expr::parse(state);

    if (!expression)
    {
      return result_type::error(expression.error());
    }

    return result_type::ok(std::make_shared<ast::record::Spread>(
      position,
      expression.value()
    ));
  }

  result_type
  parse(State& state, const ast::Position& position)
  {
    if (state.eof())
    {
      return result_type::error({
        position,
        U"Unexpected end of input; Missing record field."
      });
    }
    switch (state.current->kind())
    {
      case cst::Kind::Id:
      case cst::Kind::Str:
        return parse_named(state);

      case cst::Kind::LeftBracket:
        return parse_expr(state);

      case cst::Kind::Spread:
        return parse_spread(state);

      default:
        return result_type::error({
          state.current->position(),
          U"Unexpected " +
          cst::to_string(state.current->kind()) +
          U"; Missing record field."
        });
    }
  }
}
