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
#include <snek/ast/import.hpp>
#include <snek/parser.hpp>

namespace snek::parser::import
{
  static result_type
  parse_named_import(State& state)
  {
    const auto position = state.current->position();
    const auto name = *state.current++->text();
    std::optional<std::u32string> alias;

    if (state.peek_read(cst::Kind::KeywordAs))
    {
      if (!state.peek(cst::Kind::Id))
      {
        return result_type::error({
          position,
          U"Unexpected " +
          cst::to_string(state.current->kind()) +
          U"; Missing identifier."
        });
      }
      alias = state.current++->text();
    }

    return result_type::ok(std::make_shared<ast::import::Named>(
      position,
      name,
      alias
    ));
  }

  static result_type
  parse_star_import(State& state)
  {
    const auto position = state.current++->position();

    if (!state.peek_read(cst::Kind::KeywordAs))
    {
      return result_type::error({
        position,
        U"Missing `as' after `import *'."
      });
    }
    if (!state.peek(cst::Kind::Id))
    {
      return result_type::error({
        position,
        U"Unexpected " +
        cst::to_string(state.current->kind()) +
        U"; Missing identifier."
      });
    }

    return result_type::ok(std::make_shared<ast::import::Star>(
      position,
      *state.current++->text()
    ));
  }

  result_type
  parse(State& state, const ast::Position& position)
  {
    if (state.eof())
    {
      return result_type::error({
        position,
        U"Unexpected end of input; Missing import specifier."
      });
    }
    switch (state.current->kind())
    {
      case cst::Kind::Id:
        return parse_named_import(state);

      case cst::Kind::Mul:
        return parse_star_import(state);

      default:
        return result_type::error({
          state.current->position(),
          U"Unexpected " +
          cst::to_string(state.current->kind()) +
          U"; Missing import specifier."
        });
    }
  }
}
