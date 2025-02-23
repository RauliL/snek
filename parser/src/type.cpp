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
#include <functional>

#include "snek/parser/error.hpp"
#include "snek/parser/parameter.hpp"
#include "snek/parser/type.hpp"
#include "snek/parser/utils.hpp"

namespace snek::parser::type
{
  static ptr
  ParseMultiple(Lexer& lexer, const Token& token, const ptr& first_type)
  {
    Multiple::container_type types;

    types.push_back(first_type);
    types.push_back(Parse(lexer));
    while (lexer.PeekReadToken(token.kind))
    {
      types.push_back(Parse(lexer));
    }

    return std::make_shared<Multiple>(
      token.position,
      token.kind == Token::Kind::BitwiseAnd
        ? Multiple::MultipleKind::Intersection
        : Multiple::MultipleKind::Union,
      types
    );
  }

  static void
  ParseContainer(
    const std::optional<Position>& position,
    Lexer& lexer,
    const std::function<void()>& callback,
    Token::Kind closing_token,
    const char32_t* description
  )
  {
    for (;;)
    {
      if (lexer.PeekToken(Token::Kind::Eof))
      {
        throw SyntaxError{
          position,
          std::u32string(U"Unterminated ") +
          description +
          U" type; Missing " +
          Token::ToString(closing_token) +
          U"."
        };
      }
      else if (lexer.PeekReadToken(closing_token))
      {
        break;
      }
      callback();
      if (
        !lexer.PeekToken(Token::Kind::Comma) &&
        !lexer.PeekToken(closing_token)
      )
      {
        throw SyntaxError{
          position,
          U"Unterminated " +
          std::u32string(description) +
          U" type; Missing " +
          Token::ToString(closing_token) +
          U"."
        };
      }
      if (!lexer.PeekReadToken(Token::Kind::Comma))
      {
        lexer.ReadToken(closing_token);
        break;
      }
    }
  }

  static ptr
  ParseFunction(Lexer& lexer, const std::optional<Position>& position)
  {
    std::vector<Parameter> parameters;
    ptr return_type;

    lexer.UnreadToken(Token(position, Token::Kind::LeftParen));
    parameters = Parameter::ParseList(lexer);
    lexer.ReadToken(Token::Kind::FatArrow);
    return_type = Parse(lexer);

    return std::make_shared<Function>(position, parameters, return_type);
  }

  static ptr
  ParseRecord(Lexer& lexer, const std::optional<Position>& position)
  {
    Record::container_type fields;

    ParseContainer(
      position,
      lexer,
      [&]() -> void
      {
        const auto name = lexer.PeekToken(Token::Kind::String)
          ? *lexer.ReadToken().text
          : lexer.ReadId();

        lexer.ReadToken(Token::Kind::Colon);
        fields[name] = Parse(lexer);
      },
      Token::Kind::RightBrace,
      U"record"
    );

    return std::make_shared<Record>(position, fields);
  }

  static ptr
  ParseTuple(Lexer& lexer, const std::optional<Position>& position)
  {
    std::vector<ptr> elements;

    ParseContainer(
      position,
      lexer,
      [&]() -> void
      {
        elements.push_back(Parse(lexer));
      },
      Token::Kind::RightBracket,
      U"tuple"
    );

    return std::make_shared<Multiple>(
      position,
      Multiple::MultipleKind::Tuple,
      elements
    );
  }

  ptr
  Parse(Lexer& lexer)
  {
    const auto token = lexer.ReadToken();
    ptr type;

    switch (token.kind)
    {
      case Token::Kind::Eof:
        throw SyntaxError{
          token.position,
          U"Unexpected end of input; Missing type."
        };

      case Token::Kind::Id:
        type = std::make_shared<Named>(token.position, *token.text);
        break;

      case Token::Kind::KeywordNull:
        type = std::make_shared<Null>(token.position);
        break;

      case Token::Kind::KeywordFalse:
      case Token::Kind::KeywordTrue:
        type = std::make_shared<Boolean>(
          token.position,
          token.kind == Token::Kind::KeywordTrue
        );
        break;

      case Token::Kind::String:
        type = std::make_shared<String>(token.position, *token.text);
        break;

      case Token::Kind::LeftParen:
        type = ParseFunction(lexer, token.position);
        break;

      case Token::Kind::LeftBrace:
        type = ParseRecord(lexer, token.position);
        break;

      case Token::Kind::LeftBracket:
        type = ParseTuple(lexer, token.position);
        break;

      default:
        throw SyntaxError{
          token.position,
          U"Unexpected " + token.ToString() + U"; Missing type."
        };
    }

    for (;;)
    {
      if (lexer.PeekReadToken(Token::Kind::LeftBracket))
      {
        lexer.ReadToken(Token::Kind::RightBracket);
        type = std::make_shared<List>(type->position, type);
      }
      else if (
        lexer.PeekToken(Token::Kind::BitwiseAnd) ||
        lexer.PeekToken(Token::Kind::BitwiseOr)
      )
      {
        type = ParseMultiple(lexer, lexer.ReadToken(), type);
      } else {
        break;
      }
    }

    return type;
  }

  std::u32string
  Function::ToString() const
  {
    std::u32string result(1, U'(');
    const auto size = parameters.size();

    for (std::size_t i = 0; i < size; ++i)
    {
      if (i > 0)
      {
        result.append(U", ");
      }
      result.append(parameters[i].ToString());
    }
    result.append(U") => ");
    result.append(return_type ? return_type->ToString() : U"any");

    return result;
  }

  std::u32string
  Multiple::ToString() const
  {
    const char32_t* separator =
      multiple_kind == MultipleKind::Intersection
      ? U" & "
      : multiple_kind == MultipleKind::Union
      ? U" | "
      : U", ";
    std::u32string result;
    bool first = true;

    if (multiple_kind == MultipleKind::Tuple)
    {
      result.append(1, U'[');
    }
    for (const auto& type : types)
    {
      if (first)
      {
        first = false;
      } else {
        result.append(separator);
      }
      result.append(type->ToString());
    }
    if (multiple_kind == MultipleKind::Tuple)
    {
      result.append(1, U']');
    }

    return result;
  }

  std::u32string
  Record::ToString() const
  {
    bool first = true;
    std::u32string result(1, U'{');

    for (const auto& field : fields)
    {
      if (first)
      {
        first = false;
      } else {
        result.append(U", ");
      }
      if (utils::IsId(field.first))
      {
        result.append(field.first);
      } else {
        result.append(utils::ToJsonString(field.first));
      }
      result.append(U"; ");
      result.append(field.second->ToString());
    }
    result.append(1, U'}');

    return result;
  }

  std::u32string
  String::ToString() const
  {
    return utils::ToJsonString(value);
  }
}
