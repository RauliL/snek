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
#include <cmath>

#include <catch2/catch_test_macros.hpp>

#include "snek/parser/utils.hpp"

using namespace snek::parser::utils;

TEST_CASE("IsNewLine()")
{
  REQUIRE(IsNewLine('\r'));
  REQUIRE(IsNewLine('\n'));
  REQUIRE(!IsNewLine('a'));
  REQUIRE(!IsNewLine(' '));
}

TEST_CASE("IsIdStart()")
{
  REQUIRE(IsIdStart('a'));
  REQUIRE(IsIdStart('$'));
  REQUIRE(IsIdStart('_'));
  REQUIRE(IsIdStart(0x00e4));
  REQUIRE(!IsIdStart('0'));
}

TEST_CASE("IsIdPart()")
{
  REQUIRE(IsIdPart('a'));
  REQUIRE(IsIdPart('$'));
  REQUIRE(IsIdPart('_'));
  REQUIRE(IsIdPart(0x00e4));
  REQUIRE(IsIdPart('0'));
  REQUIRE(!IsIdPart(' '));
}

TEST_CASE("IsId()")
{
  REQUIRE(IsId(U"$a0_"));
  REQUIRE(IsId(U"_"));
  REQUIRE(IsId(U"$"));
  REQUIRE(!IsId(U"0xfoo"));
  REQUIRE(!IsId(U" foo"));
}

TEST_CASE("IsNumberPart()")
{
  REQUIRE(IsNumberPart('2'));
  REQUIRE(IsNumberPart('_'));
  REQUIRE(!IsNumberPart(' '));
  REQUIRE(!IsNumberPart('a'));
}

TEST_CASE("IntToString()")
{
  REQUIRE(!IntToString(15).compare(U"15"));
  REQUIRE(!IntToString(-5).compare(U"-5"));
}

TEST_CASE("DoubleToString()")
{
  REQUIRE(!DoubleToString(0.0).compare(U"0"));
  REQUIRE(!DoubleToString(1.5).compare(U"1.5"));
  REQUIRE(!DoubleToString(2.0).compare(U"2"));
  REQUIRE(!DoubleToString(-3.0).compare(U"-3"));
  REQUIRE(!DoubleToString(INFINITY).compare(U"Inf"));
  REQUIRE(!DoubleToString(-INFINITY).compare(U"-Inf"));
  REQUIRE(!DoubleToString(NAN).compare(U"NaN"));
}

TEST_CASE("ToJsonString()")
{
  REQUIRE(!ToJsonString(U"").compare(U"\"\""));
  REQUIRE(!ToJsonString(U"foo").compare(U"\"foo\""));
  REQUIRE(!ToJsonString(U"\b").compare(U"\"\\b\""));
  REQUIRE(!ToJsonString(U"\t").compare(U"\"\\t\""));
  REQUIRE(!ToJsonString(U"\r").compare(U"\"\\r\""));
  REQUIRE(!ToJsonString(U"\n").compare(U"\"\\n\""));
  REQUIRE(!ToJsonString(U"\f").compare(U"\"\\f\""));
  REQUIRE(!ToJsonString(U"\"\\/").compare(U"\"\\\"\\\\\\/\""));
  REQUIRE(!ToJsonString(U"\u007f").compare(U"\"\\u007f\""));
}
