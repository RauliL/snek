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
#include "snek/error.hpp"
#include "snek/parser/field.hpp"
#include "snek/parser/expression.hpp"
#include "snek/parser/utils.hpp"

namespace snek::parser::field
{
  static ptr
  ParseComputed(const Position& position, Lexer& lexer)
  {
    const auto expression = expression::Parse(lexer);

    lexer.ReadToken(Token::Kind::RightBracket);
    lexer.ReadToken(Token::Kind::Colon);

    return std::make_shared<Computed>(
      position,
      expression,
      expression::Parse(lexer)
    );
  }

  static ptr
  ParseSpread(const Position& position, Lexer& lexer)
  {
    return std::make_shared<Spread>(position, expression::Parse(lexer));
  }

  static ptr
  ParseNamed(const Token& token, Lexer& lexer)
  {
    // TODO: Figure out how to handle shorthand properties with non-identifier
    // key such as string or number.
    if (!lexer.PeekReadToken(Token::Kind::Colon))
    {
      return std::make_shared<Shorthand>(token.position(), *token.text());
    }

    return std::make_shared<Named>(
      token.position(),
      *token.text(),
      expression::Parse(lexer)
    );
  }

  ptr
  Parse(Lexer& lexer)
  {
    const auto token = lexer.ReadToken();

    switch (token.kind())
    {
      case Token::Kind::LeftBracket:
        return ParseComputed(token.position(), lexer);

      case Token::Kind::Spread:
        return ParseSpread(token.position(), lexer);

      case Token::Kind::Id:
      case Token::Kind::String:
      case Token::Kind::Float:
      case Token::Kind::Int:
        return ParseNamed(token, lexer);

      default:
        throw Error{
          lexer.position(),
          U"Unexpected " +
          token.ToString() +
          U"; Missing record field."
        };
    }
  }

  std::u32string
  Computed::ToString() const
  {
    return U'[' + m_key->ToString() + U"]: " + m_value->ToString();
  }

  std::u32string
  Named::ToString() const
  {
    if (utils::IsId(m_name))
    {
      return m_name + U": " + m_value->ToString();
    }

    return utils::ToJsonString(m_name) + U": " + m_value->ToString();
  }

  std::u32string
  Shorthand::ToString() const
  {
    return utils::IsId(m_name) ? m_name : utils::ToJsonString(m_name);
  }

  std::u32string
  Spread::ToString() const
  {
    return U"..." + m_expression->ToString();
  }
}
