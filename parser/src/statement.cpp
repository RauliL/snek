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
#include "snek/parser/statement.hpp"
#include "snek/parser/type.hpp"

namespace snek::parser::statement
{
  static inline void
  SkipNewLine(Lexer& lexer)
  {
    if (!lexer.PeekReadToken(Token::Kind::Eof))
    {
      lexer.PeekReadToken(Token::Kind::NewLine);
    }
  }

  ptr
  ParseBlock(const Position& position, Lexer& lexer)
  {
    if (lexer.PeekReadToken(Token::Kind::NewLine))
    {
      statement::Block::container_type statements;

      if (!lexer.PeekReadToken(Token::Kind::Indent))
      {
        throw Error{ position, U"Missing block." };
      }
      do
      {
        statements.push_back(Parse(lexer, false));
      }
      while (
        lexer.PeekToken(Token::Kind::Eof) ||
        lexer.PeekReadToken(Token::Kind::Dedent)
      );

      return std::make_shared<Block>(position, statements);
    }

    return Parse(lexer, false);
  }

  static ptr
  ParseExpression(Lexer& lexer, const Token& token)
  {
    expression::ptr expression;

    lexer.UnreadToken(token);
    expression = expression::Parse(lexer);
    SkipNewLine(lexer);

    return std::make_shared<Expression>(expression);
  }

  static ptr
  ParseIf(const Position& position, Lexer& lexer)
  {
    const auto condition = expression::Parse(lexer);
    ptr then_statement;
    ptr else_statement;

    lexer.PeekReadToken(Token::Kind::Colon);
    then_statement = ParseBlock(position, lexer);
    if (lexer.PeekReadToken(Token::Kind::KeywordElse))
    {
      if (lexer.PeekToken(Token::Kind::KeywordIf))
      {
        else_statement = ParseIf(lexer.ReadToken().position(), lexer);
      } else {
        lexer.ReadToken(Token::Kind::Colon);
        else_statement = ParseBlock(position, lexer);
      }
    }

    return std::make_shared<If>(
      position,
      condition,
      then_statement,
      else_statement
    );
  }

  static ptr
  ParseWhile(const Position& position, Lexer& lexer)
  {
    const auto condition = expression::Parse(lexer);

    lexer.ReadToken(Token::Kind::Colon);

    return std::make_shared<While>(
      position,
      condition,
      ParseBlock(position, lexer)
    );
  }

  static ptr
  ParseDeclareVar(const Token& token, Lexer& lexer)
  {
    const auto name = lexer.ReadId();
    expression::ptr value;

    if (lexer.PeekReadToken(Token::Kind::Assign))
    {
      value = expression::Parse(lexer);
    }
    SkipNewLine(lexer);

    return std::make_shared<DeclareVar>(
      token.position(),
      token.kind() == Token::Kind::KeywordConst,
      name,
      value
    );
  }

  static ptr
  ParseDeclareType(const Position& position, Lexer& lexer)
  {
    const auto name = lexer.ReadId();
    type::ptr type;

    lexer.ReadToken(Token::Kind::Assign);
    type = type::Parse(lexer);
    SkipNewLine(lexer);

    return std::make_shared<DeclareType>(position, name, type);
  }

  ptr
  Parse(Lexer& lexer, bool)
  {
    const auto token = lexer.ReadToken();

    switch (token.kind())
    {
      case Token::Kind::Eof:
        throw Error{
          token.position(),
          U"Unexpected end of input; Missing statement."
        };

      case Token::Kind::KeywordPass:
        SkipNewLine(lexer);

        return nullptr;

      // TODO: import statement
      // TODO: export statement

      case Token::Kind::KeywordIf:
        return ParseIf(token.position(), lexer);

      case Token::Kind::KeywordWhile:
        return ParseWhile(token.position(), lexer);

      // TODO: break statement
      // TODO: continue statement
      // TODO: return statement
      // TODO: variable declaration

      case Token::Kind::KeywordConst:
      case Token::Kind::KeywordLet:
        return ParseDeclareVar(token, lexer);

      case Token::Kind::KeywordType:
        return ParseDeclareType(token.position(), lexer);

      default:
        return ParseExpression(lexer, token);
    }
  }

  std::u32string
  DeclareType::ToString() const
  {
    return U"type " + m_name + U" = " + m_type->ToString();
  }

  std::u32string
  DeclareVar::ToString() const
  {
    std::u32string result(m_read_only ? U"const " : U"let ");

    result.append(m_name);
    if (m_value)
    {
      result.append(U" = ").append(m_value->ToString());
    }

    return result;
  }

  std::u32string
  If::ToString() const
  {
    std::u32string result(U"if ");

    result.append(m_condition->ToString()).append(U": ");
    result.append(m_then_statement->ToString());
    if (m_else_statement)
    {
      result.append(U" else: ");
      result.append(m_else_statement->ToString());
    }

    return result;
  }

  std::u32string
  Jump::ToString(JumpKind kind)
  {
    return kind == JumpKind::Break
      ? U"break"
      : kind == JumpKind::Continue
      ? U"continue"
      : U"return";
  }

  std::u32string
  Jump::ToString() const
  {
    std::u32string result(ToString(m_jump_kind));

    if (m_value)
    {
      result.append(1, U' ').append(m_value->ToString());
    }

    return result;
  }

  std::u32string
  While::ToString() const
  {
    std::u32string result(U"while ");

    result
      .append(m_condition->ToString())
      .append(U": ")
      .append(m_body->ToString());

    return result;
  }
}
