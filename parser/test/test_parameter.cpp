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
#include "snek/parser/parameter.hpp"
#include "snek/parser/type.hpp"

using namespace snek::parser;

TEST_CASE("Parse simple parameter")
{
  Lexer lexer("foo");
  const auto result = parameter::Parse(lexer);

  REQUIRE(!result->name().compare(U"foo"));
  REQUIRE(!result->type());
  REQUIRE(!result->default_value());
  REQUIRE(!result->rest());
}

TEST_CASE("Parse rest parameter")
{
  Lexer lexer("...foo");
  const auto result = parameter::Parse(lexer);

  REQUIRE(!result->name().compare(U"foo"));
  REQUIRE(!result->type());
  REQUIRE(!result->default_value());
  REQUIRE(result->rest());
}

TEST_CASE("Parse typed parameter")
{
  Lexer lexer("foo: String");
  const auto result = parameter::Parse(lexer);

  REQUIRE(!result->name().compare(U"foo"));
  REQUIRE(result->type()->kind() == type::Kind::Named);
  REQUIRE(!result->default_value());
  REQUIRE(!result->rest());
}

TEST_CASE("Parse parameter with default value")
{
  Lexer lexer("foo = bar");
  const auto result = parameter::Parse(lexer);

  REQUIRE(!result->name().compare(U"foo"));
  REQUIRE(!result->type());
  REQUIRE(result->default_value()->kind() == expression::Kind::Id);
  REQUIRE(!result->rest());
}

TEST_CASE("Parse parameter list without opening parenthesis")
{
  Lexer lexer("foo)");

  REQUIRE_THROWS_AS(parameter::ParseList(lexer), Error);
}

TEST_CASE("Parse empty parameter list")
{
  Lexer lexer("()");
  const auto result = parameter::ParseList(lexer);

  REQUIRE(result.empty());
}

TEST_CASE("Parse parameter list with one parameter")
{
  Lexer lexer("(foo)");
  const auto result = parameter::ParseList(lexer);

  REQUIRE(result.size() == 1);
  REQUIRE(!result[0]->name().compare(U"foo"));
}

TEST_CASE("Parse parameter list with two parameters")
{
  Lexer lexer("(foo, bar)");
  const auto result = parameter::ParseList(lexer);

  REQUIRE(result.size() == 2);
  REQUIRE(!result[0]->name().compare(U"foo"));
  REQUIRE(!result[1]->name().compare(U"bar"));
}

TEST_CASE("Parse parameter list with dangling comma")
{
  Lexer lexer("(foo,)");
  const auto result = parameter::ParseList(lexer);

  REQUIRE(result.size() == 1);
}

TEST_CASE("Parse unterminated parameter list")
{
  Lexer lexer("(foo");

  REQUIRE_THROWS_AS(parameter::ParseList(lexer), Error);
}

TEST_CASE("Parse unterminated parameter list with dangling comma")
{
  Lexer lexer("(foo,");

  REQUIRE_THROWS_AS(parameter::ParseList(lexer), Error);
}
