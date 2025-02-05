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

#include "snek/parser/field.hpp"

using namespace snek::parser;

TEST_CASE("Parse computed record field")
{
  Lexer lexer("[foo]: \"bar\"");
  const auto result = field::Parse(lexer);

  REQUIRE(result->kind() == field::Kind::Computed);
}

TEST_CASE("Parse function record field with block")
{
  Lexer lexer("foo(a, b):\n  pass");
  const auto result = field::Parse(lexer);

  REQUIRE(result->kind() == field::Kind::Function);
}

TEST_CASE("Parse function record field with arrow")
{
  Lexer lexer("foo(a, b) => foo");
  const auto result = field::Parse(lexer);

  REQUIRE(result->kind() == field::Kind::Function);
}

TEST_CASE("Parse named record field")
{
  Lexer lexer("\"foo\": \"bar\"");
  const auto result = field::Parse(lexer);

  REQUIRE(result->kind() == field::Kind::Named);
}

TEST_CASE("Parse shorthand record field")
{
  Lexer lexer("foo");
  const auto result = field::Parse(lexer);

  REQUIRE(result->kind() == field::Kind::Shorthand);
}

TEST_CASE("Parse spread record field")
{
  Lexer lexer("...foo");
  const auto result = field::Parse(lexer);

  REQUIRE(result->kind() == field::Kind::Spread);
}
