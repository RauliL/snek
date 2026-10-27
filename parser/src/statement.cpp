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
#include "snek/parser/field.hpp"
#include "snek/parser/import.hpp"
#include "snek/parser/statement.hpp"
#include "snek/parser/type.hpp"
#include "snek/parser/utils.hpp"

namespace snek::parser
{
  static inline void
  SkipNewLine(Lexer& lexer)
  {
    if (!lexer.PeekReadToken(TokenKind::Eof))
    {
      lexer.PeekReadToken(TokenKind::NewLine);
    }
  }

  static void
  ParseBlockLike(Lexer& lexer, const std::function<void()>& callback)
  {
    lexer.ReadToken(TokenKind::Indent);
    do
    {
      if (!lexer.PeekReadToken(TokenKind::NewLine))
      {
        callback();
      }
    }
    while (!lexer.PeekReadToken(TokenKind::Dedent));
  }

  static statement_ptr
  ParseJump(Lexer& lexer)
  {
    const auto token = lexer.ReadToken();
    expression_ptr value;

    if (
      token.kind == TokenKind::KeywordReturn &&
      !lexer.PeekToken(TokenKind::Eof) &&
      !lexer.PeekToken(TokenKind::NewLine) &&
      !lexer.PeekToken(TokenKind::Semicolon)
    )
    {
      value = Expression::Parse(lexer);
    }

    return std::make_shared<Statement::Jump>(
      token.position,
      static_cast<JumpKind>(token.kind),
      value
    );
  }

  static statement_ptr
  ParseDeclareVar(Lexer& lexer, bool exported = false)
  {
    const auto token = lexer.ReadToken();
    expression_ptr variable;
    expression_ptr value;

    if (lexer.PeekToken(TokenKind::Id))
    {
      variable = std::make_shared<Expression::Id>(
        token.position,
        *lexer.ReadToken().text
      );
      // Shorthand function declaration.
      if (lexer.PeekToken(TokenKind::LeftParen))
      {
        return std::make_shared<Statement::DeclareVar>(
          token.position,
          exported,
          token.kind == TokenKind::KeywordConst,
          variable,
          Expression::ParseFunction(lexer)
        );
      }
      // Record expression with block syntax.
      else if (lexer.PeekReadToken(TokenKind::Colon))
      {
        Expression::Record::container_type fields;

        // TODO: Support for single line record with ";".
        lexer.ReadToken(TokenKind::NewLine);
        ParseBlockLike(
          lexer,
          [&]() -> void
          {
            fields.push_back(Field::Parse(lexer));
          }
        );

        return std::make_shared<Statement::DeclareVar>(
          token.position,
          exported,
          token.kind == TokenKind::KeywordConst,
          variable,
          std::make_shared<Expression::Record>(variable->position, fields)
        );
      }
    } else {
      variable = Expression::ParseTernary(lexer);
      if (!variable->IsAssignable())
      {
        throw SyntaxError{
          variable->position,
          U"Cannot assign to " +
          variable->ToString() +
          U"."
        };
      }
    }
    if (lexer.PeekReadToken(TokenKind::Assign))
    {
      value = Expression::Parse(lexer);
    }

    return std::make_shared<Statement::DeclareVar>(
      token.position,
      exported,
      token.kind == TokenKind::KeywordConst,
      variable,
      value
    );
  }

  static statement_ptr
  ParseDeclareType(Lexer& lexer, bool exported = false)
  {
    const auto position = lexer.ReadToken().position;
    const auto name = lexer.ReadId();
    type_ptr type;

    if (lexer.PeekReadToken(TokenKind::Colon))
    {
      Type::Record::container_type fields;

      // TODO: Support for single line type with ";".
      lexer.ReadToken(TokenKind::NewLine);
      ParseBlockLike(
        lexer,
        [&]() -> void
        {
          const auto name = lexer.ReadRecordKey();

          lexer.ReadToken(TokenKind::Colon);
          fields[name] = Type::Parse(lexer);
        }
      );
      type = std::make_shared<Type::Record>(position, fields);
    } else {
      lexer.ReadToken(TokenKind::Assign);
      type = Type::Parse(lexer);
    }

    return std::make_shared<Statement::DeclareType>(
      position,
      exported,
      name,
      type
    );
  }

  static statement_ptr
  ParseImport(Lexer& lexer)
  {
    const auto position = lexer.ReadToken().position;
    Statement::Import::container_type specifiers;
    std::u32string path;

    do
    {
      // To allow dangling commas.
      if (!specifiers.empty() && lexer.PeekToken(TokenKind::KeywordFrom))
      {
        break;
      }
      specifiers.push_back(parser::Import::ParseSpecifier(lexer));
    }
    while (lexer.PeekReadToken(TokenKind::Colon));
    lexer.PeekReadToken(TokenKind::KeywordFrom);
    path = lexer.ReadString();
    SkipNewLine(lexer);

    return std::make_shared<Statement::Import>(position, specifiers, path);
  }

  static statement_ptr
  ParseExport(Lexer& lexer)
  {
    const auto token = lexer.ReadToken();
    statement_ptr statement;

    if (
      lexer.PeekToken(TokenKind::KeywordLet) ||
      lexer.PeekToken(TokenKind::KeywordConst)
    )
    {
      statement = ParseDeclareVar(lexer, true);
    }
    else if (lexer.PeekToken(TokenKind::KeywordType))
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

  static statement_ptr
  ParseSimpleStatement(Lexer& lexer, bool is_top_level)
  {
    const auto token = lexer.PeekToken();
    statement_ptr statement;

    switch (token.kind)
    {
      case TokenKind::Eof:
        throw SyntaxError{
          token.position,
          U"Unexpected end of input; Missing statement."
        };

      case TokenKind::KeywordPass:
        lexer.ReadToken();
        break;

      case TokenKind::KeywordBreak:
      case TokenKind::KeywordContinue:
      case TokenKind::KeywordReturn:
        statement = ParseJump(lexer);
        break;

      case TokenKind::KeywordConst:
      case TokenKind::KeywordLet:
        statement = ParseDeclareVar(lexer);
        break;

      case TokenKind::KeywordType:
        statement = ParseDeclareType(lexer);
        break;

      default:
        statement = std::make_shared<Statement::Expression>(
          Expression::Parse(lexer)
        );
        break;
    }

    if (
      lexer.PeekReadToken(TokenKind::Semicolon) &&
      !lexer.PeekToken(TokenKind::Eof) &&
      !lexer.PeekToken(TokenKind::NewLine)
    )
    {
      return std::make_shared<Statement::Block>(
        statement->position,
        Statement::Block::container_type{
          statement,
          ParseSimpleStatement(lexer, is_top_level)
        }
      );
    }

    SkipNewLine(lexer);

    return statement;
  }

  statement_ptr
  Statement::ParseBlock(Lexer& lexer)
  {
    if (lexer.PeekReadToken(TokenKind::NewLine))
    {
      const auto position = lexer.position();
      Statement::Block::container_type statements;

      ParseBlockLike(
        lexer,
        [&]() -> void
        {
          statements.push_back(Parse(lexer, false));
        }
      );

      return std::make_shared<Block>(position, statements);
    }

    return ParseSimpleStatement(lexer, false);
  }

  statement_ptr
  Statement::ParseFunctionBody(Lexer& lexer)
  {
    if (lexer.PeekReadToken(TokenKind::FatArrow))
    {
      const auto value = parser::Expression::Parse(lexer);

      return std::make_shared<Jump>(
        value->position,
        JumpKind::Return,
        value
      );
    }
    lexer.ReadToken(TokenKind::Colon);

    return Statement::ParseBlock(lexer);
  }

  static statement_ptr
  ParseIf(Lexer& lexer)
  {
    const auto position = lexer.ReadToken().position;
    const auto condition = Expression::Parse(lexer);
    statement_ptr then_statement;
    statement_ptr else_statement;

    lexer.ReadToken(TokenKind::Colon);
    then_statement = Statement::ParseBlock(lexer);
    if (lexer.PeekReadToken(TokenKind::KeywordElse))
    {
      if (lexer.PeekToken(TokenKind::KeywordIf))
      {
        else_statement = ParseIf(lexer);
      } else {
        lexer.ReadToken(TokenKind::Colon);
        else_statement = Statement::ParseBlock(lexer);
      }
    }

    return std::make_shared<Statement::If>(
      position,
      condition,
      then_statement,
      else_statement
    );
  }

  static statement_ptr
  ParseWhile(Lexer& lexer)
  {
    const auto position = lexer.ReadToken().position;
    const auto condition = Expression::Parse(lexer);

    lexer.ReadToken(TokenKind::Colon);

    return std::make_shared<Statement::While>(
      position,
      condition,
      Statement::ParseBlock(lexer)
    );
  }

  statement_ptr
  Statement::Parse(Lexer& lexer, bool is_top_level)
  {
    const auto token = lexer.PeekToken();

    switch (token.kind)
    {
      case TokenKind::KeywordIf:
        return ParseIf(lexer);

      case TokenKind::KeywordWhile:
        return ParseWhile(lexer);

      case TokenKind::KeywordImport:
        if (!is_top_level)
        {
          throw SyntaxError{
            token.position,
            U"Imports are only allowed at top level."
          };
        }

        return ParseImport(lexer);

      case TokenKind::KeywordExport:
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
  Statement::DeclareType::ToString() const
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
  Statement::DeclareVar::ToString() const
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
  Statement::If::ToString() const
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
  Statement::Import::ToString() const
  {
    std::u32string result(U"import ");

    result.append(
      utils::Join<value_type, container_type::const_iterator>(
        std::begin(specifiers),
        std::end(specifiers),
        [](const auto& specifier)
        {
          return specifier->ToString();
        }
      )
    );

    return result.append(U" from ").append(utils::ToJsonString(path));
  }

  std::u32string
  Statement::Jump::ToString(JumpKind kind)
  {
    return kind == JumpKind::Break
      ? U"break"
      : kind == JumpKind::Continue
      ? U"continue"
      : U"return";
  }

  std::u32string
  Statement::Jump::ToString() const
  {
    std::u32string result(ToString(jump_kind));

    if (value)
    {
      result.append(1, U' ').append(value->ToString());
    }

    return result;
  }

  std::u32string
  Statement::While::ToString() const
  {
    std::u32string result(U"while ");

    result
      .append(condition->ToString())
      .append(U": ")
      .append(body->ToString());

    return result;
  }
}
