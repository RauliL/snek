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
#include "snek/parser/import.hpp"
#include "snek/parser/statement.hpp"
#include "snek/parser/type.hpp"
#include "snek/parser/utils.hpp"

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
      token.kind == Token::Kind::KeywordReturn &&
      !lexer.PeekToken(Token::Kind::Eof) &&
      !lexer.PeekToken(Token::Kind::NewLine) &&
      !lexer.PeekToken(Token::Kind::Semicolon)
    )
    {
      value = expression::Parse(lexer);
    }

    return std::make_shared<Jump>(
      token.position,
      static_cast<JumpKind>(token.kind),
      value
    );
  }

  static ptr
  ParseDeclareVar(Lexer& lexer, bool exported = false)
  {
    const auto token = lexer.ReadToken();
    const auto variable = expression::ParseTernary(lexer);
    expression::ptr value;

    if (!variable->IsAssignable())
    {
      throw SyntaxError{
        variable->position,
        U"Cannot assign to " +
        variable->ToString() +
        U"."
      };
    }
    if (lexer.PeekReadToken(Token::Kind::Assign))
    {
      value = expression::Parse(lexer);
    }

    return std::make_shared<DeclareVar>(
      token.position,
      exported,
      token.kind == Token::Kind::KeywordConst,
      variable,
      value
    );
  }

  static ptr
  ParseDeclareType(Lexer& lexer, bool exported = false)
  {
    const auto position = lexer.ReadToken().position;
    const auto name = lexer.ReadId();

    lexer.ReadToken(Token::Kind::Assign);

    return std::make_shared<DeclareType>(
      position,
      exported,
      name,
      type::Parse(lexer)
    );
  }

  static ptr
  ParseImport(Lexer& lexer)
  {
    const auto position = lexer.ReadToken().position;
    Import::container_type specifiers;
    std::u32string path;

    do
    {
      // To allow dangling commas.
      if (!specifiers.empty() && lexer.PeekToken(Token::Kind::KeywordFrom))
      {
        break;
      }
      specifiers.push_back(import::ParseSpecifier(lexer));
    }
    while (lexer.PeekReadToken(Token::Kind::Colon));
    lexer.PeekReadToken(Token::Kind::KeywordFrom);
    path = lexer.ReadString();
    SkipNewLine(lexer);

    return std::make_shared<Import>(position, specifiers, path);
  }

  static ptr
  ParseExport(Lexer& lexer)
  {
    const auto token = lexer.ReadToken();
    ptr statement;

    if (
      lexer.PeekToken(Token::Kind::KeywordLet) ||
      lexer.PeekToken(Token::Kind::KeywordConst)
    )
    {
      statement = ParseDeclareVar(lexer, true);
    }
    else if (lexer.PeekToken(Token::Kind::KeywordType))
    {
      statement = ParseDeclareType(lexer, true);
    } else {
      throw SyntaxError{
        token.position,
        U"Unexpected " + lexer.PeekToken().ToString() + U" after `export'."
      };
    }
    SkipNewLine(lexer);

    return statement;
  }

  static ptr
  ParseSimpleStatement(Lexer& lexer, bool is_top_level)
  {
    const auto token = lexer.PeekToken();
    ptr statement;

    switch (token.kind)
    {
      case Token::Kind::Eof:
        throw SyntaxError{
          token.position,
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
        statement->position,
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
        value->position,
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
    const auto position = lexer.ReadToken().position;
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
    const auto position = lexer.ReadToken().position;
    const auto condition = expression::Parse(lexer);

    lexer.ReadToken(Token::Kind::Colon);

    return std::make_shared<While>(position, condition, ParseBlock(lexer));
  }

  ptr
  Parse(Lexer& lexer, bool is_top_level)
  {
    const auto token = lexer.PeekToken();

    switch (token.kind)
    {
      case Token::Kind::KeywordIf:
        return ParseIf(lexer);

      case Token::Kind::KeywordWhile:
        return ParseWhile(lexer);

      case Token::Kind::KeywordImport:
        if (!is_top_level)
        {
          throw SyntaxError{
            token.position,
            U"Imports are only allowed at top level."
          };
        }

        return ParseImport(lexer);

      case Token::Kind::KeywordExport:
        if (!is_top_level)
        {
          throw SyntaxError{
            token.position,
            U"Exports are only allowed at top level."
          };
        }

        return ParseExport(lexer);

      default:
        return ParseSimpleStatement(lexer, is_top_level);
    }
  }

  std::u32string
  DeclareType::ToString() const
  {
    std::u32string result;

    if (is_export)
    {
      result.append(U"export ");
    }
    return result
      .append(U"type ")
      .append(name)
      .append(U" = ")
      .append(type->ToString());
  }

  std::u32string
  DeclareVar::ToString() const
  {
    std::u32string result;

    if (is_export)
    {
      result.append(U"export ");
    }
    result
      .append(is_read_only ? U"const " : U"let ")
      .append(variable->ToString());
    if (value)
    {
      result.append(U" = ").append(value->ToString());
    }

    return result;
  }

  std::u32string
  If::ToString() const
  {
    std::u32string result(U"if ");

    result.append(condition->ToString()).append(U": ");
    result.append(then_statement->ToString());
    if (else_statement)
    {
      result.append(U" else: ").append(else_statement->ToString());
    }

    return result;
  }

  std::u32string
  Import::ToString() const
  {
    std::u32string result(U"import ");
    const auto size = specifiers.size();

    for (std::size_t i = 0; i < size; ++i)
    {
      if (i > 0)
      {
        result.append(U", ");
      }
      result.append(specifiers[i]->ToString());
    }

    return result.append(U" from ").append(utils::ToJsonString(path));
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
    std::u32string result(ToString(jump_kind));

    if (value)
    {
      result.append(1, U' ').append(value->ToString());
    }

    return result;
  }

  std::u32string
  While::ToString() const
  {
    std::u32string result(U"while ");

    result
      .append(condition->ToString())
      .append(U": ")
      .append(body->ToString());

    return result;
  }
}
