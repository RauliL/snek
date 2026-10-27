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
#include "snek/parser/signature.hpp"
#include "snek/parser/utils.hpp"
#include "snek/parser/type.hpp"

namespace snek::parser
{
  Parameter
  Parameter::Parse(Lexer& lexer)
  {
    const auto position = lexer.position();
    const auto rest = lexer.PeekReadToken(TokenKind::Spread);
    const auto name = lexer.ReadId();
    type_ptr type;
    expression_ptr default_value;

    if (lexer.PeekReadToken(TokenKind::Colon))
    {
      type = Type::Parse(lexer);
    }
    if (lexer.PeekReadToken(TokenKind::Assign))
    {
      default_value = Expression::Parse(lexer);
    }

    return Parameter(position, name, type, default_value, rest);
  }

  std::vector<Parameter>
  Parameter::ParseList(Lexer& lexer, bool read_opening_parenthesis)
  {
    std::vector<Parameter> result;

    if (read_opening_parenthesis)
    {
      lexer.ReadToken(TokenKind::LeftParen);
    }

    for (;;)
    {
      if (lexer.PeekReadToken(TokenKind::RightParen))
      {
        break;
      }

      const auto parameter = Parse(lexer);

      result.push_back(parameter);

      if (
        !lexer.PeekToken(TokenKind::RightParen) &&
        !lexer.PeekToken(TokenKind::Comma)
      )
      {
        throw SyntaxError{
          lexer.position(),
          U"Unterminated parameter list; Missing `)'."
        };
      }
      lexer.PeekReadToken(TokenKind::Comma);
      if (parameter.rest)
      {
        lexer.ReadToken(TokenKind::RightParen);
        break;
      }
    }

    return result;
  }

  std::u32string
  Parameter::ToString() const
  {
    std::u32string result;

    if (rest)
    {
      result.append(U"...");
    }
    result.append(name);
    if (type)
    {
      result.append(U": ").append(type->ToString());
    }
    if (default_value)
    {
      result.append(U" = ").append(default_value->ToString());
    }

    return result;
  }

  std::u32string
  Signature::ToString(
    const std::u32string& return_type_separator,
    bool include_missing_return_type
  ) const
  {
    auto result = utils::Join<
      Parameter,
      std::vector<Parameter>::const_iterator
    >(
      std::begin(parameters),
      std::end(parameters),
      [](const auto& parameter)
      {
        return parameter.ToString();
      },
      U'(',
      U')'
    );

    if (return_type)
    {
      result.append(return_type_separator).append(return_type->ToString());
    }
    else if (include_missing_return_type)
    {
      result.append(return_type_separator).append(U"any");
    }

    return result;
  }
}
