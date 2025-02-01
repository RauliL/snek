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
#include "snek/parser/expression.hpp"
#include "snek/parser/parameter.hpp"
#include "snek/parser/type.hpp"

namespace snek::parser::parameter
{
  ptr
  Parse(Lexer& lexer)
  {
    const auto position = lexer.position();
    std::u32string name;
    type::ptr type;
    expression::ptr default_value;

    if (!lexer.PeekToken(Token::Kind::Id))
    {
      throw Error{
        position,
        U"Unexpected "
        + lexer.ReadToken().ToString()
        + U"; Missing identifier."
      };
    }
    name = *lexer.ReadToken().text();
    if (lexer.PeekReadToken(Token::Kind::Colon))
    {
      type = type::Parse(lexer);
    }
    if (lexer.PeekReadToken(Token::Kind::Assign))
    {
      default_value = expression::Parse(lexer);
    }

    return std::make_shared<Base>(position, name, type, default_value);
  }

  std::vector<ptr>
  ParseList(Lexer& lexer, bool read_opening_parenthesis)
  {
    std::vector<ptr> result;

    if (read_opening_parenthesis)
    {
      lexer.ReadToken(Token::Kind::LeftParen);
    }

    for (;;)
    {
      if (lexer.PeekReadToken(Token::Kind::RightParen))
      {
        break;
      }
      result.push_back(Parse(lexer));
      if (
        lexer.PeekToken(Token::Kind::RightParen) &&
        lexer.PeekToken(Token::Kind::Comma)
      )
      {
        throw Error{
          lexer.position(),
          U"Unterminated parameter list; Missing `)'."
        };
      }
      lexer.PeekReadToken(Token::Kind::Comma);
    }

    return result;
  }

  std::u32string
  Base::ToString() const
  {
    std::u32string result(m_name);

    if (m_type)
    {
      result.append(U": ").append(m_type->ToString());
    }
    if (m_default_value)
    {
      result.append(U" = ").append(m_default_value->ToString());
    }

    return result;
  }
}
