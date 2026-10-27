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
#include "snek/parser/type.hpp"

using namespace snek::parser;

TEST_CASE("Hitting end of input while parsing a type")
{
  Lexer lexer("");

  REQUIRE_THROWS_AS(Type::Parse(lexer), SyntaxError);
}

TEST_CASE("Parse named type")
{
  Lexer lexer("foo");
  const auto result = Type::Parse(lexer);

  REQUIRE(result->Is(TypeKind::Named));
}

TEST_CASE("Parse null type")
{
  Lexer lexer("null");
  const auto result = Type::Parse(lexer);

  REQUIRE(result->Is(TypeKind::Null));
}

TEST_CASE("Parse boolean type: true")
{
  Lexer lexer("true");
  const auto result = Type::Parse(lexer);

  REQUIRE(result->Is(TypeKind::Boolean));
  REQUIRE(As<Type::Boolean>(result)->value);
}

TEST_CASE("Parse boolean type: false")
{
  Lexer lexer("false");
  const auto result = Type::Parse(lexer);

  REQUIRE(result->Is(TypeKind::Boolean));
  REQUIRE(!As<Type::Boolean>(result)->value);
}

TEST_CASE("Parse string type")
{
  Lexer lexer("\"foo\"");
  const auto result = Type::Parse(lexer);

  REQUIRE(result->Is(TypeKind::String));
  REQUIRE(!As<Type::String>(result)->value.compare(U"foo"));
}

TEST_CASE("Parse function type")
{
  Lexer lexer("(foo) => bar");
  const auto result = Type::Parse(lexer);
  const Type::Function* function;

  REQUIRE(result->Is(TypeKind::Function));
  function = As<Type::Function>(result);
  REQUIRE(function->signature.parameters.size() == 1);
  REQUIRE(function->signature.return_type->Is(TypeKind::Named));
}

TEST_CASE("Parse empty record type")
{
  Lexer lexer("{}");
  const auto result = Type::Parse(lexer);

  REQUIRE(result->Is(TypeKind::Record));
  REQUIRE(As<Type::Record>(result)->fields.empty());
}

TEST_CASE("Parse record type with one field")
{
  Lexer lexer("{ foo: bar }");
  const auto result = Type::Parse(lexer);

  REQUIRE(result->Is(TypeKind::Record));
  REQUIRE(As<Type::Record>(result)->fields.size() == 1);
}

TEST_CASE("Parse record type with one field and dangling comma")
{
  Lexer lexer("{ foo: bar, }");
  const auto result = Type::Parse(lexer);

  REQUIRE(result->Is(TypeKind::Record));
  REQUIRE(As<Type::Record>(result)->fields.size() == 1);
}

TEST_CASE("Parse record type with string as field name")
{
  Lexer lexer("{ \"foo\": bar }");
  const auto result = Type::Parse(lexer);

  REQUIRE(result->Is(TypeKind::Record));
  REQUIRE(As<Type::Record>(result)->fields.size() == 1);
}

TEST_CASE("Parse record type with two fields")
{
  Lexer lexer("{ foo: bar, bar: foo }");
  const auto result = Type::Parse(lexer);

  REQUIRE(result->Is(TypeKind::Record));
  REQUIRE(As<Type::Record>(result)->fields.size() == 2);
}

TEST_CASE("Parse unterminated record type")
{
  Lexer lexer("{ \"foo\": bar");

  REQUIRE_THROWS_AS(Type::Parse(lexer), SyntaxError);
}

TEST_CASE("Parse empty tuple type")
{
  Lexer lexer("[]");
  const auto result = Type::Parse(lexer);
  const Type::Multiple* multiple;

  REQUIRE(result->Is(TypeKind::Multiple));
  multiple = As<Type::Multiple>(result);
  REQUIRE(multiple->Is(Type::Multiple::MultipleKind::Tuple));
  REQUIRE(multiple->types.empty());
}

TEST_CASE("Parse tuple type with one element")
{
  Lexer lexer("[foo]");
  const auto result = Type::Parse(lexer);
  const Type::Multiple* multiple;

  REQUIRE(result->Is(TypeKind::Multiple));
  multiple = As<Type::Multiple>(result);
  REQUIRE(multiple->Is(Type::Multiple::MultipleKind::Tuple));
  REQUIRE(multiple->types.size() == 1);
}

TEST_CASE("Parse tuple type with one element and dangling comma")
{
  Lexer lexer("[foo,]");
  const auto result = Type::Parse(lexer);
  const Type::Multiple* multiple;

  REQUIRE(result->Is(TypeKind::Multiple));
  multiple = As<Type::Multiple>(result);
  REQUIRE(multiple->Is(Type::Multiple::MultipleKind::Tuple));
  REQUIRE(multiple->types.size() == 1);
}

TEST_CASE("Parse tuple type with two elements")
{
  Lexer lexer("[foo, foo]");
  const auto result = Type::Parse(lexer);
  const Type::Multiple* multiple;

  REQUIRE(result->Is(TypeKind::Multiple));
  multiple = As<Type::Multiple>(result);
  REQUIRE(multiple->Is(Type::Multiple::MultipleKind::Tuple));
  REQUIRE(multiple->types.size() == 2);
}

TEST_CASE("Parse unterminated tuple type")
{
  Lexer lexer("[foo,");

  REQUIRE_THROWS_AS(Type::Parse(lexer), SyntaxError);
}

TEST_CASE("Parse unrecognized type")
{
  Lexer lexer("+");

  REQUIRE_THROWS_AS(Type::Parse(lexer), SyntaxError);
}

TEST_CASE("Parse list type")
{
  Lexer lexer("foo[]");
  const auto result = Type::Parse(lexer);

  REQUIRE(result->Is(TypeKind::List));
}

TEST_CASE("Parse list type without closing bracket")
{
  Lexer lexer("foo[");

  REQUIRE_THROWS_AS(Type::Parse(lexer), SyntaxError);
}

TEST_CASE("Parse nested list type")
{
  Lexer lexer("foo[][]");
  const auto result = Type::Parse(lexer);

  REQUIRE(result->Is(TypeKind::List));
  REQUIRE(As<Type::List>(result)->element_type->Is(TypeKind::List));
}

TEST_CASE("Parse intersection type")
{
  Lexer lexer("foo & bar");
  const auto result = Type::Parse(lexer);
  const Type::Multiple* multiple;

  REQUIRE(result->Is(TypeKind::Multiple));
  multiple = As<Type::Multiple>(result);
  REQUIRE(multiple->Is(Type::Multiple::MultipleKind::Intersection));
}

TEST_CASE("Parse union type")
{
  Lexer lexer("foo | bar");
  const auto result = Type::Parse(lexer);
  const Type::Multiple* multiple;

  REQUIRE(result->Is(TypeKind::Multiple));
  multiple = As<Type::Multiple>(result);
  REQUIRE(multiple->Is(Type::Multiple::MultipleKind::Union));
}
