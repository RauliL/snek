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
#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>

#include <peelo/unicode/ctype/isspace.hpp>

namespace snek::parser::utils
{
  inline bool
  IsBlank(const std::u32string& s)
  {
    using peelo::unicode::ctype::isspace;

    return s.empty() || std::all_of(std::begin(s), std::end(s), isspace);
  }

  inline bool
  IsNewLine(char32_t c)
  {
    return c == U'\r' || c == U'\n';
  }

  bool IsIdStart(char32_t c);

  bool IsIdPart(char32_t c);

  bool IsId(const std::u32string& text);

  inline bool
  IsNumberPart(char32_t c)
  {
    return c == U'_' || std::isdigit(c);
  }

  std::u32string IntToString(std::int64_t value);

  std::u32string DoubleToString(double value);

  std::u32string ToJsonString(const std::u32string& input);
}
