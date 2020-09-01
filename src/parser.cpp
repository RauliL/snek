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
#include <snek/ast/parameter.hpp>
#include <snek/parser.hpp>

namespace snek::parser
{
  result_type
  parse(const std::vector<cst::Token>& tokens)
  {
    State state =
    {
      std::begin(tokens),
      std::end(tokens),
    };
    std::vector<std::shared_ptr<ast::stmt::Base>> statements;

    while (!state.eof())
    {
      const auto statement = stmt::parse(state);

      if (!statement)
      {
        return result_type::error(statement.error());
      }
      statements.push_back(statement.value());
    }

    return result_type::ok(statements);
  }

  parameter_result_type
  parse_parameter(State& state)
  {
    ast::Position position;
    std::u32string name;

    if (state.eof())
    {
      return parameter_result_type::error({
        std::nullopt,
        U"Unexpected end of input; Missing parameter."
      });
    }
    position = state.current->position();
    if (!state.peek(cst::Kind::Id))
    {
      return parameter_result_type::error({
        position,
        U"Unexpected" +
        cst::to_string(state.current->kind()) +
        U"; Missing parameter name."
      });
    }
    name = *state.current++->text();
    if (!state.peek_read(cst::Kind::Colon))
    {
      return parameter_result_type::error({
        position,
        U"Missing `:' after parameter declaration."
      });
    }

    const auto type = type::parse(state, position);

    if (!type)
    {
      return parameter_result_type::error(type.error());
    }

    return parameter_result_type::ok(std::make_shared<ast::Parameter>(
      position,
      name,
      type.value()
    ));
  }

  parameter_list_result_type
  parse_parameter_list(State& state)
  {
    ast::Position position;
    std::vector<std::shared_ptr<ast::Parameter>> list;

    if (state.eof())
    {
      return parameter_list_result_type::error({
        std::nullopt,
        U"Unexpected end of input; Missing parameter list."
      });
    }
    position = state.current->position();
    if (!state.peek_read(cst::Kind::LeftParen))
    {
      return parameter_list_result_type::error({
        position,
        U"Unexpected " +
        cst::to_string(state.current->kind()) +
        U"; Missing parameter list."
      });
    }
    for (;;)
    {
      if (state.eof())
      {
        return parameter_list_result_type::error({
          position,
          U"Unterminated parameter list; Missing `)'."
        });
      }
      else if (state.peek_read(cst::Kind::RightParen))
      {
        break;
      } else {
        const auto parameter = parse_parameter(state);

        if (!parameter)
        {
          return parameter_list_result_type::error(parameter.error());
        }
        list.push_back(parameter.value());
        if (!state.peek(cst::Kind::Comma) &&
            !state.peek(cst::Kind::RightParen))
        {
          return parameter_list_result_type::error({
            parameter.value()->position(),
            U"Unterminated parameter list; Missing `)'."
          });
        }
        state.peek_read(cst::Kind::Comma);
      }
    }

    return parameter_list_result_type::ok(list);
  }
}
