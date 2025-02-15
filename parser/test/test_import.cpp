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

#include "snek/parser/import.hpp"

using namespace snek::parser;

TEST_CASE("Parse ordinary import specifier")
{
  Lexer lexer("foo");
  const auto result = import::ParseSpecifier(lexer);

  REQUIRE(result->kind() == import::Kind::Named);
  REQUIRE(!result->alias);
}

TEST_CASE("Parse ordinary import specifier with alias")
{
  Lexer lexer("foo as bar");
  const auto result = import::ParseSpecifier(lexer);

  REQUIRE(result->kind() == import::Kind::Named);
  REQUIRE(!result->alias->compare(U"bar"));
}

TEST_CASE("Parse star import specifier")
{
  Lexer lexer("*");
  const auto result = import::ParseSpecifier(lexer);

  REQUIRE(result->kind() == import::Kind::Star);
  REQUIRE(!result->alias);
}

TEST_CASE("Parse star import specifier with alias")
{
  Lexer lexer("* as foo");
  const auto result = import::ParseSpecifier(lexer);

  REQUIRE(result->kind() == import::Kind::Star);
  REQUIRE(!result->alias->compare(U"foo"));
}
