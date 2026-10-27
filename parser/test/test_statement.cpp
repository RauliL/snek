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
#include <catch2/catch_test_macros.hpp>

#include "snek/parser/error.hpp"
#include "snek/parser/import.hpp"
#include "snek/parser/statement.hpp"
#include "snek/parser/type.hpp"

using namespace snek::parser;

template<class T>
static inline std::shared_ptr<T>
As(const statement_ptr& statement)
{
  return std::static_pointer_cast<T>(statement);
}

TEST_CASE("Parse pass statement")
{
  Lexer lexer("pass");
  const auto result = Statement::Parse(lexer, false);

  REQUIRE(!result);
}

TEST_CASE("Parse expression statement")
{
  Lexer lexer("foo");
  const auto result = Statement::Parse(lexer, false);

  REQUIRE(result->kind == StatementKind::Expression);
  REQUIRE(As<Statement::Expression>(result)->expression->kind ==
    ExpressionKind::Id);
}

TEST_CASE("Parse block statement from semicolon-separated statements")
{
  Lexer lexer("foo; bar");
  const auto result = Statement::Parse(lexer, false);
  std::shared_ptr<Statement::Block> block;

  REQUIRE(result->kind == StatementKind::Block);
  block = As<Statement::Block>(result);
  REQUIRE(block->statements.size() == 2);
  REQUIRE(block->statements[0]->kind == StatementKind::Expression);
  REQUIRE(block->statements[1]->kind == StatementKind::Expression);
}

TEST_CASE("Parse indented block statement")
{
  Lexer lexer(":\n  foo\n  bar\n");
  const auto result = Statement::ParseFunctionBody(lexer);
  std::shared_ptr<Statement::Block> block;

  REQUIRE(result->kind == StatementKind::Block);
  block = As<Statement::Block>(result);
  REQUIRE(block->statements.size() == 2);
}

TEST_CASE("Parse type declaration")
{
  Lexer lexer("type Foo = Bar");
  const auto result = Statement::Parse(lexer, false);
  std::shared_ptr<Statement::DeclareType> declaration;

  REQUIRE(result->kind == StatementKind::DeclareType);
  declaration = As<Statement::DeclareType>(result);
  REQUIRE(!declaration->is_export);
  REQUIRE(!declaration->name.compare(U"Foo"));
  REQUIRE(declaration->type->kind == TypeKind::Named);
}

TEST_CASE("Parse type declaration with record body")
{
  Lexer lexer("type Foo:\n  bar: Int\n");
  const auto result = Statement::Parse(lexer, false);
  std::shared_ptr<Statement::DeclareType> declaration;

  REQUIRE(result->kind == StatementKind::DeclareType);
  declaration = As<Statement::DeclareType>(result);
  REQUIRE(!declaration->name.compare(U"Foo"));
  REQUIRE(declaration->type->kind == TypeKind::Record);
}

TEST_CASE("Parse exported type declaration")
{
  Lexer lexer("export type Foo = Bar");
  const auto result = Statement::Parse(lexer, true);
  std::shared_ptr<Statement::DeclareType> declaration;

  REQUIRE(result->kind == StatementKind::DeclareType);
  declaration = As<Statement::DeclareType>(result);
  REQUIRE(declaration->is_export);
  REQUIRE(!declaration->name.compare(U"Foo"));
}

TEST_CASE("Parse variable declaration without value")
{
  Lexer lexer("let foo");
  const auto result = Statement::Parse(lexer, false);
  std::shared_ptr<Statement::DeclareVar> declaration;

  REQUIRE(result->kind == StatementKind::DeclareVar);
  declaration = As<Statement::DeclareVar>(result);
  REQUIRE(!declaration->is_export);
  REQUIRE(!declaration->is_read_only);
  REQUIRE(declaration->variable->kind == ExpressionKind::Id);
  REQUIRE(!declaration->value);
}

TEST_CASE("Parse constant declaration with value")
{
  Lexer lexer("const foo = bar");
  const auto result = Statement::Parse(lexer, false);
  std::shared_ptr<Statement::DeclareVar> declaration;

  REQUIRE(result->kind == StatementKind::DeclareVar);
  declaration = As<Statement::DeclareVar>(result);
  REQUIRE(!declaration->is_export);
  REQUIRE(declaration->is_read_only);
  REQUIRE(declaration->variable->kind == ExpressionKind::Id);
  REQUIRE(declaration->value->kind == ExpressionKind::Id);
}

TEST_CASE("Parse shorthand function declaration")
{
  Lexer lexer("const foo():\n  pass\n");
  const auto result = Statement::Parse(lexer, false);
  std::shared_ptr<Statement::DeclareVar> declaration;

  REQUIRE(result->kind == StatementKind::DeclareVar);
  declaration = As<Statement::DeclareVar>(result);
  REQUIRE(declaration->is_read_only);
  REQUIRE(declaration->value->kind == ExpressionKind::Function);
}

TEST_CASE("Parse variable declaration with record body")
{
  Lexer lexer("const foo:\n  bar: 1\n");
  const auto result = Statement::Parse(lexer, false);
  std::shared_ptr<Statement::DeclareVar> declaration;

  REQUIRE(result->kind == StatementKind::DeclareVar);
  declaration = As<Statement::DeclareVar>(result);
  REQUIRE(declaration->value->kind == ExpressionKind::Record);
}

TEST_CASE("Parse exported variable declaration")
{
  Lexer lexer("export let foo = 1");
  const auto result = Statement::Parse(lexer, true);
  std::shared_ptr<Statement::DeclareVar> declaration;

  REQUIRE(result->kind == StatementKind::DeclareVar);
  declaration = As<Statement::DeclareVar>(result);
  REQUIRE(declaration->is_export);
  REQUIRE(!declaration->is_read_only);
}

TEST_CASE("Parse if statement without else")
{
  Lexer lexer("if true: foo");
  const auto result = Statement::Parse(lexer, false);
  std::shared_ptr<Statement::If> statement;

  REQUIRE(result->kind == StatementKind::If);
  statement = As<Statement::If>(result);
  REQUIRE(statement->condition->kind == ExpressionKind::Boolean);
  REQUIRE(statement->then_statement->kind == StatementKind::Expression);
  REQUIRE(!statement->else_statement);
}

TEST_CASE("Parse if statement with else")
{
  Lexer lexer("if true: foo\nelse: bar");
  const auto result = Statement::Parse(lexer, false);
  std::shared_ptr<Statement::If> statement;

  REQUIRE(result->kind == StatementKind::If);
  statement = As<Statement::If>(result);
  REQUIRE(statement->then_statement->kind == StatementKind::Expression);
  REQUIRE(statement->else_statement->kind == StatementKind::Expression);
}

TEST_CASE("Parse if statement with else if")
{
  Lexer lexer("if true: foo\nelse if false: bar");
  const auto result = Statement::Parse(lexer, false);
  std::shared_ptr<Statement::If> statement;

  REQUIRE(result->kind == StatementKind::If);
  statement = As<Statement::If>(result);
  REQUIRE(statement->else_statement->kind == StatementKind::If);
}

TEST_CASE("Parse if statement with indented body")
{
  Lexer lexer("if true:\n  foo\n  bar\n");
  const auto result = Statement::Parse(lexer, false);
  std::shared_ptr<Statement::If> statement;

  REQUIRE(result->kind == StatementKind::If);
  statement = As<Statement::If>(result);
  REQUIRE(statement->then_statement->kind == StatementKind::Block);
}

TEST_CASE("Parse import statement")
{
  Lexer lexer("import foo from \"bar\"");
  const auto result = Statement::Parse(lexer, true);
  std::shared_ptr<Statement::Import> statement;

  REQUIRE(result->kind == StatementKind::Import);
  statement = As<Statement::Import>(result);
  REQUIRE(statement->specifiers.size() == 1);
  REQUIRE(statement->specifiers[0]->kind == ImportKind::Named);
  REQUIRE(!statement->path.compare(U"bar"));
}

TEST_CASE("Parse import statement with multiple specifiers")
{
  Lexer lexer("import foo: * as bar from \"baz\"");
  const auto result = Statement::Parse(lexer, true);
  std::shared_ptr<Statement::Import> statement;

  REQUIRE(result->kind == StatementKind::Import);
  statement = As<Statement::Import>(result);
  REQUIRE(statement->specifiers.size() == 2);
  REQUIRE(statement->specifiers[0]->kind == ImportKind::Named);
  REQUIRE(statement->specifiers[1]->kind == ImportKind::Star);
}

TEST_CASE("Reject import statement outside top level")
{
  Lexer lexer("import foo from \"bar\"");

  REQUIRE_THROWS_AS(Statement::Parse(lexer, false), SyntaxError);
}

TEST_CASE("Parse break statement")
{
  Lexer lexer("break");
  const auto result = Statement::Parse(lexer, false);
  std::shared_ptr<Statement::Jump> statement;

  REQUIRE(result->kind == StatementKind::Jump);
  statement = As<Statement::Jump>(result);
  REQUIRE(statement->jump_kind == JumpKind::Break);
  REQUIRE(!statement->value);
}

TEST_CASE("Parse continue statement")
{
  Lexer lexer("continue");
  const auto result = Statement::Parse(lexer, false);
  std::shared_ptr<Statement::Jump> statement;

  REQUIRE(result->kind == StatementKind::Jump);
  statement = As<Statement::Jump>(result);
  REQUIRE(statement->jump_kind == JumpKind::Continue);
  REQUIRE(!statement->value);
}

TEST_CASE("Parse return statement without value")
{
  Lexer lexer("return");
  const auto result = Statement::Parse(lexer, false);
  std::shared_ptr<Statement::Jump> statement;

  REQUIRE(result->kind == StatementKind::Jump);
  statement = As<Statement::Jump>(result);
  REQUIRE(statement->jump_kind == JumpKind::Return);
  REQUIRE(!statement->value);
}

TEST_CASE("Parse return statement with value")
{
  Lexer lexer("return foo");
  const auto result = Statement::Parse(lexer, false);
  std::shared_ptr<Statement::Jump> statement;

  REQUIRE(result->kind == StatementKind::Jump);
  statement = As<Statement::Jump>(result);
  REQUIRE(statement->jump_kind == JumpKind::Return);
  REQUIRE(statement->value->kind == ExpressionKind::Id);
}

TEST_CASE("Parse while statement")
{
  Lexer lexer("while true: foo");
  const auto result = Statement::Parse(lexer, false);
  std::shared_ptr<Statement::While> statement;

  REQUIRE(result->kind == StatementKind::While);
  statement = As<Statement::While>(result);
  REQUIRE(statement->condition->kind == ExpressionKind::Boolean);
  REQUIRE(statement->body->kind == StatementKind::Expression);
}

TEST_CASE("Parse while statement with indented body")
{
  Lexer lexer("while true:\n  foo\n  break\n");
  const auto result = Statement::Parse(lexer, false);
  std::shared_ptr<Statement::While> statement;

  REQUIRE(result->kind == StatementKind::While);
  statement = As<Statement::While>(result);
  REQUIRE(statement->body->kind == StatementKind::Block);
}

TEST_CASE("Parse function body with fat arrow")
{
  Lexer lexer("=> foo");
  const auto result = Statement::ParseFunctionBody(lexer);
  std::shared_ptr<Statement::Jump> statement;

  REQUIRE(result->kind == StatementKind::Jump);
  statement = As<Statement::Jump>(result);
  REQUIRE(statement->jump_kind == JumpKind::Return);
  REQUIRE(statement->value->kind == ExpressionKind::Id);
}

TEST_CASE("Parse function body with block")
{
  Lexer lexer(": foo");
  const auto result = Statement::ParseFunctionBody(lexer);

  REQUIRE(result->kind == StatementKind::Expression);
}

TEST_CASE("Reject export statement outside top level")
{
  Lexer lexer("export let foo = 1");

  REQUIRE_THROWS_AS(Statement::Parse(lexer, false), SyntaxError);
}

TEST_CASE("Hitting end of input while parsing a statement")
{
  Lexer lexer("");

  REQUIRE_THROWS_AS(Statement::Parse(lexer, false), SyntaxError);
}
