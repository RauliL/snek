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

  static ptr
  ParseJump(Lexer& lexer)
  {
    const auto token = lexer.ReadToken();
    expression::ptr value;

    if (
      token.kind() == Token::Kind::KeywordReturn &&
      !lexer.PeekToken(Token::Kind::Eof) &&
      !lexer.PeekToken(Token::Kind::NewLine) &&
      !lexer.PeekToken(Token::Kind::Semicolon)
    )
    {
      value = expression::Parse(lexer);
    }

    return std::make_shared<Jump>(
      token.position(),
      static_cast<JumpKind>(token.kind()),
      value
    );
  }

  static ptr
  ParseDeclareVar(Lexer& lexer)
  {
    const auto token = lexer.ReadToken();
    const auto name = lexer.ReadId();
    expression::ptr value;

    if (lexer.PeekReadToken(Token::Kind::Assign))
    {
      value = expression::Parse(lexer);
    }

    return std::make_shared<DeclareVar>(
      token.position(),
      token.kind() == Token::Kind::KeywordConst,
      name,
      value
    );
  }

  static ptr
  ParseDeclareType(Lexer& lexer)
  {
    const auto position = lexer.ReadToken().position();
    const auto name = lexer.ReadId();

    lexer.ReadToken(Token::Kind::Assign);

    return std::make_shared<DeclareType>(position, name, type::Parse(lexer));
  }

  static ptr
  ParseSimpleStatement(Lexer& lexer, bool is_top_level)
  {
    const auto token = lexer.PeekToken();
    ptr statement;

    switch (token.kind())
    {
      case Token::Kind::Eof:
        throw Error{
          token.position(),
          U"Unexpected end of input; Missing statement."
        };

      case Token::Kind::KeywordPass:
        lexer.ReadToken();
        break;

      case Token::Kind::KeywordBreak:
      case Token::Kind::KeywordContinue:
      case Token::Kind::KeywordReturn:
        statement = ParseJump(lexer);
        break;

      // TODO: import statement
      // TODO: export statement

      case Token::Kind::KeywordConst:
      case Token::Kind::KeywordLet:
        statement = ParseDeclareVar(lexer);
        break;

      case Token::Kind::KeywordType:
        statement = ParseDeclareType(lexer);
        break;

      default:
        statement = std::make_shared<Expression>(expression::Parse(lexer));
        break;
    }

    if (
      lexer.PeekReadToken(Token::Kind::Semicolon) &&
      !lexer.PeekToken(Token::Kind::Eof) &&
      !lexer.PeekToken(Token::Kind::NewLine)
    )
    {
      return std::make_shared<Block>(
        statement->position(),
        Block::container_type{
          statement,
          ParseSimpleStatement(lexer, is_top_level)
        }
      );
    }

    SkipNewLine(lexer);

    return statement;
  }

  ptr
  ParseBlock(Lexer& lexer)
  {
    if (lexer.PeekReadToken(Token::Kind::NewLine))
    {
      const auto position = lexer.position();
      statement::Block::container_type statements;

      lexer.ReadToken(Token::Kind::Indent);
      do
      {
        if (!lexer.PeekReadToken(Token::Kind::NewLine))
        {
          statements.push_back(Parse(lexer, false));
        }
      }
      while (!lexer.PeekReadToken(Token::Kind::Dedent));

      return std::make_shared<Block>(position, statements);
    }

    return ParseSimpleStatement(lexer, false);
  }

  ptr
  ParseFunctionBody(Lexer& lexer)
  {
    if (lexer.PeekReadToken(Token::Kind::FatArrow))
    {
      const auto value = expression::Parse(lexer);

      return std::make_shared<Jump>(
        value->position(),
        JumpKind::Return,
        value
      );
    }
    lexer.ReadToken(Token::Kind::Colon);

    return ParseBlock(lexer);
  }

  static ptr
  ParseIf(Lexer& lexer)
  {
    const auto position = lexer.ReadToken().position();
    const auto condition = expression::Parse(lexer);
    ptr then_statement;
    ptr else_statement;

    lexer.ReadToken(Token::Kind::Colon);
    then_statement = ParseBlock(lexer);
    if (lexer.PeekReadToken(Token::Kind::KeywordElse))
    {
      if (lexer.PeekToken(Token::Kind::KeywordIf))
      {
        else_statement = ParseIf(lexer);
      } else {
        lexer.ReadToken(Token::Kind::Colon);
        else_statement = ParseBlock(lexer);
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
  ParseWhile(Lexer& lexer)
  {
    const auto position = lexer.ReadToken().position();
    const auto condition = expression::Parse(lexer);

    lexer.ReadToken(Token::Kind::Colon);

    return std::make_shared<While>(position, condition, ParseBlock(lexer));
  }

  ptr
  Parse(Lexer& lexer, bool is_top_level)
  {
    const auto token = lexer.PeekToken();

    switch (token.kind())
    {
      case Token::Kind::KeywordIf:
        return ParseIf(lexer);

      case Token::Kind::KeywordWhile:
        return ParseWhile(lexer);

      default:
        return ParseSimpleStatement(lexer, is_top_level);
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
