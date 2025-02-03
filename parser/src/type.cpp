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
    while (lexer.PeekReadToken(token.kind()))
    {
      types.push_back(Parse(lexer));
    }

    return std::make_shared<Multiple>(
      token.position(),
      token.kind() == Token::Kind::BitwiseAnd
        ? Multiple::MultipleKind::Intersection
        : Multiple::MultipleKind::Union,
      types
    );
  }

  static ptr
  ParseFunction(Lexer& lexer, const Position& position)
  {
    std::vector<parameter::ptr> parameters;
    ptr return_type;

    lexer.UnreadToken(Token(position, Token::Kind::LeftParen));
    parameters = parameter::ParseList(lexer);
    lexer.ReadToken(Token::Kind::FatArrow);
    return_type = Parse(lexer);

    return std::make_shared<Function>(position, parameters, return_type);
  }

  static ptr
  ParseRecord(Lexer& lexer, const Position& position)
  {
    Record::container_type fields;

    for (;;)
    {
      std::u32string name;
      ptr value;

      if (lexer.PeekToken(Token::Kind::Eof))
      {
        throw Error{
          position,
          U"Unterminated record type; Missing `}'."
        };
      }
      else if (lexer.PeekReadToken(Token::Kind::RightBrace))
      {
        break;
      }
      if (lexer.PeekToken(Token::Kind::String))
      {
        name = *lexer.ReadToken().text();
      } else {
        name = lexer.ReadId();
      }
      lexer.ReadToken(Token::Kind::Colon);
      value = Parse(lexer);
      fields[name] = value;
      if (
        !lexer.PeekToken(Token::Kind::Comma) &&
        !lexer.PeekToken(Token::Kind::RightBrace)
      )
      {
        throw Error{
          position,
          U"Unterminated record type; Missing `}'."
        };
      }
      if (!lexer.PeekReadToken(Token::Kind::Comma))
      {
        lexer.ReadToken(Token::Kind::RightBrace);
        break;
      }
    }

    return std::make_shared<Record>(position, fields);
  }

  static ptr
  ParseTuple(Lexer& lexer, const Position& position)
  {
    std::vector<ptr> elements;

    for (;;)
    {
      if (lexer.PeekToken(Token::Kind::Eof))
      {
        throw Error{
          position,
          U"Unterminated tuple type; Missing `]'."
        };
      }
      else if (lexer.PeekReadToken(Token::Kind::RightBracket))
      {
        break;
      }
      elements.push_back(Parse(lexer));
      if (
        !lexer.PeekToken(Token::Kind::Comma) &&
        !lexer.PeekToken(Token::Kind::RightBracket)
      )
      {
        throw Error{
          position,
          U"Unterminated tuple type; Missing `]'."
        };
      }
      if (!lexer.PeekReadToken(Token::Kind::Comma))
      {
        lexer.ReadToken(Token::Kind::RightBracket);
        break;
      }
    }

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

    switch (token.kind())
    {
      case Token::Kind::Eof:
        throw Error{
          token.position(),
          U"Unexpected end of input; Missing type."
        };

      case Token::Kind::Id:
        type = std::make_shared<Named>(token.position(), *token.text());
        break;

      case Token::Kind::KeywordNull:
        type = std::make_shared<Null>(token.position());
        break;

      case Token::Kind::KeywordFalse:
      case Token::Kind::KeywordTrue:
        type = std::make_shared<Boolean>(
          token.position(),
          token.kind() == Token::Kind::KeywordTrue
        );
        break;

      case Token::Kind::String:
        type = std::make_shared<String>(token.position(), *token.text());
        break;

      case Token::Kind::LeftParen:
        type = ParseFunction(lexer, token.position());
        break;

      case Token::Kind::LeftBrace:
        type = ParseRecord(lexer, token.position());
        break;

      case Token::Kind::LeftBracket:
        type = ParseTuple(lexer, token.position());
        break;

      default:
        throw Error{
          token.position(),
          U"Unexpected " + token.ToString() + U"; Missing type."
        };
    }

    for (;;)
    {
      if (lexer.PeekReadToken(Token::Kind::LeftBracket))
      {
        lexer.ReadToken(Token::Kind::RightBracket);
        type = std::make_shared<List>(type->position(), type);
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

    for (std::size_t i = 0; i < m_parameters.size(); ++i)
    {
      if (i > 0)
      {
        result.append(U", ");
      }
      result.append(m_parameters[i]->ToString());
    }
    result.append(U") => ");
    result.append(m_return_type ? m_return_type->ToString() : U"null");

    return result;
  }

  std::u32string
  Multiple::ToString() const
  {
    const char32_t* separator =
      m_multiple_kind == MultipleKind::Intersection
      ? U" & "
      : m_multiple_kind == MultipleKind::Union
      ? U" | "
      : U", ";
    std::u32string result;
    bool first = true;

    if (m_multiple_kind == MultipleKind::Tuple)
    {
      result.append(1, U'[');
    }
    for (const auto& type : m_types)
    {
      if (first)
      {
        first = false;
      } else {
        result.append(separator);
      }
      result.append(type->ToString());
    }
    if (m_multiple_kind == MultipleKind::Tuple)
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

    for (const auto& field : m_fields)
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
    return utils::ToJsonString(m_value);
  }
}
