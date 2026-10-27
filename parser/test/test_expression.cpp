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
#include "snek/parser/expression.hpp"
#include "snek/parser/statement.hpp"

using namespace snek::parser;

template<class T>
static inline std::shared_ptr<T>
As(const expression_ptr& expression)
{
  return std::static_pointer_cast<T>(expression);
}

TEST_CASE("Hitting end of input while parsing an expression")
{
  Lexer lexer("");

  REQUIRE_THROWS_AS(Expression::Parse(lexer), SyntaxError);
}

TEST_CASE("Parse null expression")
{
  Lexer lexer("null");
  const auto result = Expression::Parse(lexer);

  REQUIRE(result->kind == ExpressionKind::Null);
}

TEST_CASE("Parse boolean expression: true")
{
  Lexer lexer("true");
  const auto result = Expression::Parse(lexer);

  REQUIRE(result->kind == ExpressionKind::Boolean);
  REQUIRE(As<Expression::Boolean>(result)->value);
}

TEST_CASE("Parse boolean expression: false")
{
  Lexer lexer("false");
  const auto result = Expression::Parse(lexer);

  REQUIRE(result->kind == ExpressionKind::Boolean);
  REQUIRE(!As<Expression::Boolean>(result)->value);
}

TEST_CASE("Parse int expression")
{
  Lexer lexer("42");
  const auto result = Expression::Parse(lexer);

  REQUIRE(result->kind == ExpressionKind::Int);
  REQUIRE(As<Expression::Int>(result)->value == 42);
}

TEST_CASE("Parse float expression")
{
  Lexer lexer("3.14");
  const auto result = Expression::Parse(lexer);

  REQUIRE(result->kind == ExpressionKind::Float);
  REQUIRE(As<Expression::Float>(result)->value == 3.14);
}

TEST_CASE("Parse string expression")
{
  Lexer lexer("\"foo\"");
  const auto result = Expression::Parse(lexer);

  REQUIRE(result->kind == ExpressionKind::String);
  REQUIRE(!As<Expression::String>(result)->value.compare(U"foo"));
}

TEST_CASE("Parse id expression")
{
  Lexer lexer("foo");
  const auto result = Expression::Parse(lexer);

  REQUIRE(result->kind == ExpressionKind::Id);
  REQUIRE(!As<Expression::Id>(result)->identifier.compare(U"foo"));
}

TEST_CASE("Parse parenthesized expression")
{
  Lexer lexer("(foo)");
  const auto result = Expression::Parse(lexer);

  REQUIRE(result->kind == ExpressionKind::Id);
}

TEST_CASE("Parse empty list expression")
{
  Lexer lexer("[]");
  const auto result = Expression::Parse(lexer);

  REQUIRE(result->kind == ExpressionKind::List);
  REQUIRE(As<Expression::List>(result)->elements.empty());
}

TEST_CASE("Parse list expression with elements")
{
  Lexer lexer("[foo, bar]");
  const auto result = Expression::Parse(lexer);

  REQUIRE(result->kind == ExpressionKind::List);
  REQUIRE(As<Expression::List>(result)->elements.size() == 2);
}

TEST_CASE("Parse empty record expression")
{
  Lexer lexer("{}");
  const auto result = Expression::Parse(lexer);

  REQUIRE(result->kind == ExpressionKind::Record);
  REQUIRE(As<Expression::Record>(result)->fields.empty());
}

TEST_CASE("Parse record expression with fields")
{
  Lexer lexer("{ foo: bar, baz }");
  const auto result = Expression::Parse(lexer);

  REQUIRE(result->kind == ExpressionKind::Record);
  REQUIRE(As<Expression::Record>(result)->fields.size() == 2);
}

TEST_CASE("Parse function expression with fat arrow")
{
  Lexer lexer("() => foo");
  const auto result = Expression::Parse(lexer);
  std::shared_ptr<Expression::Function> function;

  REQUIRE(result->kind == ExpressionKind::Function);
  function = As<Expression::Function>(result);
  REQUIRE(function->signature.parameters.empty());
  REQUIRE(function->body->kind == StatementKind::Jump);
}

TEST_CASE("Parse function expression with parameter and return type")
{
  Lexer lexer("(foo: Int) -> Int => foo");
  const auto result = Expression::Parse(lexer);
  std::shared_ptr<Expression::Function> function;

  REQUIRE(result->kind == ExpressionKind::Function);
  function = As<Expression::Function>(result);
  REQUIRE(function->signature.parameters.size() == 1);
  REQUIRE(function->signature.return_type);
}

TEST_CASE("Parse function expression with block body")
{
  Lexer lexer("():\n  pass\n");
  const auto result = Expression::Parse(lexer);

  REQUIRE(result->kind == ExpressionKind::Function);
}

TEST_CASE("Parse single-parameter function without parameter list")
{
  Lexer lexer("(foo) => bar");
  const auto result = Expression::Parse(lexer);
  std::shared_ptr<Expression::Function> function;

  REQUIRE(result->kind == ExpressionKind::Function);
  function = As<Expression::Function>(result);
  REQUIRE(function->signature.parameters.size() == 1);
}

TEST_CASE("Parse unary not expression")
{
  Lexer lexer("!foo");
  const auto result = Expression::Parse(lexer);
  std::shared_ptr<Expression::Unary> unary;

  REQUIRE(result->kind == ExpressionKind::Unary);
  unary = As<Expression::Unary>(result);
  REQUIRE(unary->op == UnaryOp::Not);
  REQUIRE(unary->operand->kind == ExpressionKind::Id);
}

TEST_CASE("Parse unary plus expression")
{
  Lexer lexer("+foo");
  const auto result = Expression::Parse(lexer);

  REQUIRE(result->kind == ExpressionKind::Unary);
  REQUIRE(As<Expression::Unary>(result)->op == UnaryOp::Add);
}

TEST_CASE("Parse unary minus expression")
{
  Lexer lexer("-foo");
  const auto result = Expression::Parse(lexer);

  REQUIRE(result->kind == ExpressionKind::Unary);
  REQUIRE(As<Expression::Unary>(result)->op == UnaryOp::Sub);
}

TEST_CASE("Parse bitwise not expression")
{
  Lexer lexer("~foo");
  const auto result = Expression::Parse(lexer);

  REQUIRE(result->kind == ExpressionKind::Unary);
  REQUIRE(As<Expression::Unary>(result)->op == UnaryOp::BitwiseNot);
}

TEST_CASE("Parse prefix increment expression")
{
  Lexer lexer("++foo");
  const auto result = Expression::Parse(lexer);
  std::shared_ptr<Expression::Increment> increment;

  REQUIRE(result->kind == ExpressionKind::Increment);
  increment = As<Expression::Increment>(result);
  REQUIRE(increment->pre);
  REQUIRE(increment->variable->kind == ExpressionKind::Id);
}

TEST_CASE("Parse postfix increment expression")
{
  Lexer lexer("foo++");
  const auto result = Expression::Parse(lexer);
  std::shared_ptr<Expression::Increment> increment;

  REQUIRE(result->kind == ExpressionKind::Increment);
  increment = As<Expression::Increment>(result);
  REQUIRE(!increment->pre);
}

TEST_CASE("Parse prefix decrement expression")
{
  Lexer lexer("--foo");
  const auto result = Expression::Parse(lexer);
  std::shared_ptr<Expression::Decrement> decrement;

  REQUIRE(result->kind == ExpressionKind::Decrement);
  decrement = As<Expression::Decrement>(result);
  REQUIRE(decrement->pre);
}

TEST_CASE("Parse postfix decrement expression")
{
  Lexer lexer("foo--");
  const auto result = Expression::Parse(lexer);
  std::shared_ptr<Expression::Decrement> decrement;

  REQUIRE(result->kind == ExpressionKind::Decrement);
  decrement = As<Expression::Decrement>(result);
  REQUIRE(!decrement->pre);
}

TEST_CASE("Parse property expression")
{
  Lexer lexer("foo.bar");
  const auto result = Expression::Parse(lexer);
  std::shared_ptr<Expression::Property> property;

  REQUIRE(result->kind == ExpressionKind::Property);
  property = As<Expression::Property>(result);
  REQUIRE(property->expression->kind == ExpressionKind::Id);
  REQUIRE(!property->name.compare(U"bar"));
  REQUIRE(!property->conditional);
}

TEST_CASE("Parse conditional property expression")
{
  Lexer lexer("foo?.bar");
  const auto result = Expression::Parse(lexer);
  std::shared_ptr<Expression::Property> property;

  REQUIRE(result->kind == ExpressionKind::Property);
  property = As<Expression::Property>(result);
  REQUIRE(property->conditional);
  REQUIRE(!property->name.compare(U"bar"));
}

TEST_CASE("Parse call expression")
{
  Lexer lexer("foo(bar, baz)");
  const auto result = Expression::Parse(lexer);
  std::shared_ptr<Expression::Call> call;

  REQUIRE(result->kind == ExpressionKind::Call);
  call = As<Expression::Call>(result);
  REQUIRE(call->expression->kind == ExpressionKind::Id);
  REQUIRE(call->arguments.size() == 2);
  REQUIRE(!call->conditional);
}

TEST_CASE("Parse conditional call expression")
{
  Lexer lexer("foo?.(bar)");
  const auto result = Expression::Parse(lexer);
  std::shared_ptr<Expression::Call> call;

  REQUIRE(result->kind == ExpressionKind::Call);
  call = As<Expression::Call>(result);
  REQUIRE(call->conditional);
  REQUIRE(call->arguments.size() == 1);
}

TEST_CASE("Parse call expression with spread argument")
{
  Lexer lexer("foo(...bar)");
  const auto result = Expression::Parse(lexer);
  std::shared_ptr<Expression::Call> call;

  REQUIRE(result->kind == ExpressionKind::Call);
  call = As<Expression::Call>(result);
  REQUIRE(call->arguments.size() == 1);
  REQUIRE(call->arguments[0]->kind == ExpressionKind::Spread);
  REQUIRE(
    As<Expression::Spread>(call->arguments[0])->expression->kind ==
    ExpressionKind::Id
  );
}

TEST_CASE("Parse subscript expression")
{
  Lexer lexer("foo[bar]");
  const auto result = Expression::Parse(lexer);
  std::shared_ptr<Expression::Subscript> subscript;

  REQUIRE(result->kind == ExpressionKind::Subscript);
  subscript = As<Expression::Subscript>(result);
  REQUIRE(subscript->expression->kind == ExpressionKind::Id);
  REQUIRE(subscript->index->kind == ExpressionKind::Id);
  REQUIRE(!subscript->conditional);
}

TEST_CASE("Parse conditional subscript expression")
{
  Lexer lexer("foo?.[bar]");
  const auto result = Expression::Parse(lexer);
  std::shared_ptr<Expression::Subscript> subscript;

  REQUIRE(result->kind == ExpressionKind::Subscript);
  subscript = As<Expression::Subscript>(result);
  REQUIRE(subscript->conditional);
}

TEST_CASE("Parse binary expression")
{
  Lexer lexer("foo + bar");
  const auto result = Expression::Parse(lexer);
  std::shared_ptr<Expression::Binary> binary;

  REQUIRE(result->kind == ExpressionKind::Binary);
  binary = As<Expression::Binary>(result);
  REQUIRE(binary->left->kind == ExpressionKind::Id);
  REQUIRE(binary->op == BinaryOp::Add);
  REQUIRE(binary->right->kind == ExpressionKind::Id);
}

TEST_CASE("Parse binary equality expression")
{
  Lexer lexer("foo == bar");
  const auto result = Expression::Parse(lexer);

  REQUIRE(result->kind == ExpressionKind::Binary);
  REQUIRE(As<Expression::Binary>(result)->op == BinaryOp::Equal);
}

TEST_CASE("Parse binary logical and expression")
{
  Lexer lexer("foo && bar");
  const auto result = Expression::Parse(lexer);

  REQUIRE(result->kind == ExpressionKind::Binary);
  REQUIRE(As<Expression::Binary>(result)->op == BinaryOp::LogicalAnd);
}

TEST_CASE("Parse binary null coalescing expression")
{
  Lexer lexer("foo ?? bar");
  const auto result = Expression::Parse(lexer);

  REQUIRE(result->kind == ExpressionKind::Binary);
  REQUIRE(As<Expression::Binary>(result)->op == BinaryOp::NullCoalescing);
}

TEST_CASE("Parse ternary expression")
{
  Lexer lexer("foo ? bar : baz");
  const auto result = Expression::Parse(lexer);
  std::shared_ptr<Expression::Ternary> ternary;

  REQUIRE(result->kind == ExpressionKind::Ternary);
  ternary = As<Expression::Ternary>(result);
  REQUIRE(ternary->condition->kind == ExpressionKind::Id);
  REQUIRE(ternary->then_expression->kind == ExpressionKind::Id);
  REQUIRE(ternary->else_expression->kind == ExpressionKind::Id);
}

TEST_CASE("Parse assignment expression")
{
  Lexer lexer("foo = bar");
  const auto result = Expression::Parse(lexer);
  std::shared_ptr<Expression::Assign> assign;

  REQUIRE(result->kind == ExpressionKind::Assign);
  assign = As<Expression::Assign>(result);
  REQUIRE(assign->variable->kind == ExpressionKind::Id);
  REQUIRE(assign->value->kind == ExpressionKind::Id);
  REQUIRE(!assign->op);
}

TEST_CASE("Parse compound assignment expression")
{
  Lexer lexer("foo += bar");
  const auto result = Expression::Parse(lexer);
  std::shared_ptr<Expression::Assign> assign;

  REQUIRE(result->kind == ExpressionKind::Assign);
  assign = As<Expression::Assign>(result);
  REQUIRE(assign->op);
  REQUIRE(*assign->op == AssignOp::Add);
}

TEST_CASE("Reject assignment to non-assignable expression")
{
  Lexer lexer("1 = foo");

  REQUIRE_THROWS_AS(Expression::Parse(lexer), SyntaxError);
}

TEST_CASE("Parse list destructuring assignment")
{
  Lexer lexer("[foo, bar] = baz");
  const auto result = Expression::Parse(lexer);
  std::shared_ptr<Expression::Assign> assign;

  REQUIRE(result->kind == ExpressionKind::Assign);
  assign = As<Expression::Assign>(result);
  REQUIRE(assign->variable->kind == ExpressionKind::List);
}

TEST_CASE("Parse function through ParseFunction")
{
  Lexer lexer("(foo) => bar");
  const auto result = Expression::ParseFunction(lexer);

  REQUIRE(result->kind == ExpressionKind::Function);
}
