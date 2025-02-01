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

#include <peelo/unicode/ctype/isalnum.hpp>
#include <peelo/unicode/ctype/isalpha.hpp>
#include <peelo/unicode/ctype/iscntrl.hpp>
#include <peelo/unicode/encoding/utf8.hpp>

#include "snek/parser/utils.hpp"

namespace snek::parser::utils
{
  bool
  IsIdStart(char32_t c)
  {
    return c == '$' || c == '_' || peelo::unicode::ctype::isalpha(c);
  }

  bool
  IsIdPart(char32_t c)
  {
    return c == '$' || c == '_' || peelo::unicode::ctype::isalnum(c);
  }

  bool
  IsId(const std::u32string& text)
  {
    const auto length = text.length();

    if (length < 1 || !IsIdStart(text[0]))
    {
      return false;
    }
    for (std::u32string::size_type i = 1; i < length; ++i)
    {
      if (!IsIdPart(text[i]))
      {
        return false;
      }
    }

    return true;
  }

  std::u32string
  IntToString(std::int64_t value)
  {
    static const char digitmap[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    const bool negative = value < 0;
    std::uint64_t mag = static_cast<std::uint64_t>(negative ? -value : value);
    std::u32string result;

    if (mag != 0)
    {
      result.reserve(negative ? 21 : 20);
      do
      {
        result.insert(result.begin(), digitmap[mag % 10]);
        mag /= 10;
      }
      while (mag);
    } else {
      result.insert(result.begin(), '0');
    }
    if (negative)
    {
      result.insert(result.begin(), '-');
    }

    return result;
  }

  std::u32string
  DoubleToString(double value)
  {
    using peelo::unicode::encoding::utf8::decode;

    static const std::u32string string_nan = U"NaN";
    static const std::u32string string_inf = U"Inf";
    static const std::u32string string_inf_neg = U"-Inf";
    char buffer[20];

    if (std::isnan(value))
    {
      return string_nan;
    }
    else if (std::isinf(value))
    {
      return value < 0.0 ? string_inf_neg : string_inf;
    }
    std::snprintf(buffer, sizeof(buffer), "%g", value);

    return decode(buffer);
  }

  std::u32string
  ToJsonString(const std::u32string& input)
  {
    using peelo::unicode::ctype::iscntrl;

    std::u32string result;

    result.reserve(input.length() + 2);
    result.append(1, U'"');
    for (const auto& c : input)
    {
      switch (c)
      {
        case 010:
          result.append(1, '\\');
          result.append(1, 'b');
          break;

        case 011:
          result.append(1, '\\');
          result.append(1, 't');
          break;

        case 012:
          result.append(1, '\\');
          result.append(1, 'n');
          break;

        case 014:
          result.append(1, '\\');
          result.append(1, 'f');
          break;

        case 015:
          result.append(1, '\\');
          result.append(1, 'r');
          break;

        case '"':
        case '\\':
        case '/':
          result.append(1, '\\');
          result.append(1, c);
          break;

        default:
          if (iscntrl(c))
          {
            char buffer[7];

            std::snprintf(buffer, 7, "\\u%04x", static_cast<unsigned int>(c));
            for (const char* p = buffer; *p; ++p)
            {
              result.append(1, static_cast<char32_t>(*p));
            }
          } else {
            result.append(1, c);
          }
      }
    }
    result.append(1, U'"');

    return result;
  }
}
