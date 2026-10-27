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
#include "snek/parser/type.hpp"
#include "snek/parser/utils.hpp"

namespace snek::parser
{
  static type_ptr
  ParseMultiple(Lexer& lexer, const Token& token, const type_ptr& first_type)
  {
    Type::Multiple::container_type types;

    types.push_back(first_type);
    types.push_back(Type::Parse(lexer));
    while (lexer.PeekReadToken(token.kind))
    {
      types.push_back(Type::Parse(lexer));
    }

    return std::make_shared<Type::Multiple>(
      token.position,
      token.kind == TokenKind::BitwiseAnd
        ? Type::Multiple::MultipleKind::Intersection
        : Type::Multiple::MultipleKind::Union,
      types
    );
  }

  static void
  ParseContainer(
    const std::optional<Position>& position,
    Lexer& lexer,
    const std::function<void()>& callback,
    TokenKind closing_token,
    const char32_t* description
  )
  {
    for (;;)
    {
      if (lexer.PeekToken(TokenKind::Eof))
      {
        throw SyntaxError{
          position,
          std::u32string(U"Unterminated ") +
          description +
          U" type; Missing " +
          ToString(closing_token) +
          U"."
        };
      }
      else if (lexer.PeekReadToken(closing_token))
      {
        break;
      }
      callback();
      if (
        !lexer.PeekToken(TokenKind::Comma) &&
        !lexer.PeekToken(closing_token)
      )
      {
        throw SyntaxError{
          position,
          U"Unterminated " +
          std::u32string(description) +
          U" type; Missing " +
          ToString(closing_token) +
          U"."
        };
      }
      if (!lexer.PeekReadToken(TokenKind::Comma))
      {
        lexer.ReadToken(closing_token);
        break;
      }
    }
  }

  static type_ptr
  ParseFunction(Lexer& lexer, const std::optional<Position>& position)
  {
    std::vector<Parameter> parameters;
    type_ptr return_type;

    parameters = Parameter::ParseList(lexer, false);
    lexer.ReadToken(TokenKind::FatArrow);
    return_type = Type::Parse(lexer);

    return std::make_shared<Type::Function>(
      position,
      Signature{ parameters, return_type }
    );
  }

  static type_ptr
  ParseRecord(Lexer& lexer, const std::optional<Position>& position)
  {
    Type::Record::container_type fields;

    ParseContainer(
      position,
      lexer,
      [&]() -> void
      {
        const auto name = lexer.ReadRecordKey();

        lexer.ReadToken(TokenKind::Colon);
        fields[name] = Type::Parse(lexer);
      },
      TokenKind::RightBrace,
      U"record"
    );

    return std::make_shared<Type::Record>(position, fields);
  }

  static type_ptr
  ParseTuple(Lexer& lexer, const std::optional<Position>& position)
  {
    Type::Multiple::container_type elements;

    ParseContainer(
      position,
      lexer,
      [&]() -> void
      {
        elements.push_back(Type::Parse(lexer));
      },
      TokenKind::RightBracket,
      U"tuple"
    );

    return std::make_shared<Type::Multiple>(
      position,
      Type::Multiple::MultipleKind::Tuple,
      elements
    );
  }

  type_ptr
  Type::Parse(Lexer& lexer)
  {
    const auto token = lexer.ReadToken();
    type_ptr type;

    switch (token.kind)
    {
      case TokenKind::Eof:
        throw SyntaxError{
          token.position,
          U"Unexpected end of input; Missing type."
        };

      case TokenKind::Id:
        type = std::make_shared<Type::Named>(token.position, *token.text);
        break;

      case TokenKind::KeywordNull:
        type = std::make_shared<Type::Null>(token.position);
        break;

      case TokenKind::KeywordFalse:
      case TokenKind::KeywordTrue:
        type = std::make_shared<Type::Boolean>(
          token.position,
          token.kind == TokenKind::KeywordTrue
        );
        break;

      case TokenKind::String:
        type = std::make_shared<Type::String>(token.position, *token.text);
        break;

      case TokenKind::LeftParen:
        type = ParseFunction(lexer, token.position);
        break;

      case TokenKind::LeftBrace:
        type = ParseRecord(lexer, token.position);
        break;

      case TokenKind::LeftBracket:
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
      if (lexer.PeekReadToken(TokenKind::LeftBracket))
      {
        lexer.ReadToken(TokenKind::RightBracket);
        type = std::make_shared<Type::List>(type->position, type);
      }
      else if (
        lexer.PeekToken(TokenKind::BitwiseAnd) ||
        lexer.PeekToken(TokenKind::BitwiseOr)
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
  Type::Multiple::ToString() const
  {
    const char32_t* separator =
      multiple_kind == MultipleKind::Intersection
      ? U" & "
      : multiple_kind == MultipleKind::Union
      ? U" | "
      : U", ";

    return utils::Join<value_type, container_type::const_iterator>(
      std::begin(types),
      std::end(types),
      [](const auto& type)
      {
        return type->ToString();
      },
      multiple_kind == MultipleKind::Tuple
        ? std::make_optional(U'[')
        : std::nullopt,
      multiple_kind == MultipleKind::Tuple
        ? std::make_optional(U']')
        : std::nullopt,
      separator
    );
  }

  std::u32string
  Type::Record::ToString() const
  {
    return utils::Join<value_type, container_type::const_iterator>(
      std::begin(fields),
      std::end(fields),
      [](const auto& field)
      {
        return (
          utils::IsId(field.first)
            ? field.first
            : utils::ToJsonString(field.first)
        )
          + U": "
          + field.second->ToString();
      },
      U'{',
      U'}'
    );
  }

  std::u32string
  Type::String::ToString() const
  {
    return utils::ToJsonString(value);
  }
}
