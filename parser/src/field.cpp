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
#include "snek/parser/error.hpp"
#include "snek/parser/expression.hpp"
#include "snek/parser/field.hpp"
#include "snek/parser/statement.hpp"
#include "snek/parser/type.hpp"
#include "snek/parser/utils.hpp"

namespace snek::parser
{
  static field_ptr
  ParseComputed(const std::optional<Position>& position, Lexer& lexer)
  {
    const auto expression = Expression::Parse(lexer);

    lexer.ReadToken(TokenKind::RightBracket);
    lexer.ReadToken(TokenKind::Colon);

    return std::make_shared<Field::Computed>(
      position,
      expression,
      Expression::Parse(lexer)
    );
  }

  static inline field_ptr
  ParseSpread(const std::optional<Position>& position, Lexer& lexer)
  {
    return std::make_shared<Field::Spread>(position, Expression::Parse(lexer));
  }

  static field_ptr
  ParseNamed(const Token& token, Lexer& lexer)
  {
    if (lexer.PeekReadToken(TokenKind::LeftParen))
    {
      const auto parameters = Parameter::ParseList(lexer, false);
      type_ptr return_type;

      if (lexer.PeekReadToken(TokenKind::Arrow))
      {
        return_type = Type::Parse(lexer);
      }

      return std::make_shared<Field::Function>(
        token.position,
        *token.text,
        Signature{ parameters, return_type },
        Statement::ParseFunctionBody(lexer)
      );
    }
    // TODO: Figure out how to handle shorthand properties with non-identifier
    // key such as string or number.
    else if (!lexer.PeekReadToken(TokenKind::Colon))
    {
      return std::make_shared<Field::Shorthand>(token.position, *token.text);
    }

    return std::make_shared<Field::Named>(
      token.position,
      *token.text,
      Expression::Parse(lexer)
    );
  }

  field_ptr
  Field::Parse(Lexer& lexer)
  {
    const auto token = lexer.ReadToken();

    switch (token.kind)
    {
      case TokenKind::LeftBracket:
        return ParseComputed(token.position, lexer);

      case TokenKind::Spread:
        return ParseSpread(token.position, lexer);

      case TokenKind::Id:
      case TokenKind::String:
      case TokenKind::Float:
      case TokenKind::Int:
        return ParseNamed(token, lexer);

      default:
        throw SyntaxError{
          lexer.position(),
          U"Unexpected " +
          token.ToString() +
          U"; Missing record field."
        };
    }
  }

  std::u32string
  Field::Computed::ToString() const
  {
    return U'[' + key->ToString() + U"]: " + value->ToString();
  }

  std::u32string
  Field::Named::ToString() const
  {
    if (utils::IsId(name))
    {
      return name + U": " + value->ToString();
    }

    return utils::ToJsonString(name) + U": " + value->ToString();
  }

  std::u32string
  Field::Shorthand::ToString() const
  {
    return utils::IsId(name) ? name : utils::ToJsonString(name);
  }

  std::u32string
  Field::Spread::ToString() const
  {
    return U"..." + expression->ToString();
  }
}
