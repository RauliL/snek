/*
 * Copyright (c) 2020, Rauli Laine
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
#include <peelo/unicode/ctype/iscntrl.hpp>

#include <snek/cst.hpp>

namespace snek::cst
{
  std::u32string
  stringify(const std::u32string& text)
  {
    using peelo::unicode::ctype::iscntrl;
    std::u32string result;

    result.reserve(text.length() + 2);
    result.append(1, U'"');
    for (const auto& c : text)
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

            std::snprintf(buffer, 7, "\\u%04x", c);
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
  std::u32string
  to_string(Kind kind)
  {
    switch (kind)
    {
      case Kind::EndOfInput:
        return U"end of input";

      case Kind::NewLine:
        return U"new line";

      case Kind::Indent:
        return U"indentation";

      case Kind::Dedent:
        return U"dedentation";

      case Kind::Id:
        return U"identifier";

      case Kind::Float:
        return U"float";

      case Kind::Int:
        return U"int";

      case Kind::Str:
        return U"string";

      case Kind::KeywordAs:
        return U"`as'";

      case Kind::KeywordBreak:
        return U"`break'";

      case Kind::KeywordContinue:
        return U"`continue'";

      case Kind::KeywordElse:
        return U"`else'";

      case Kind::KeywordExport:
        return U"`export'";

      case Kind::KeywordFalse:
        return U"`false'";

      case Kind::KeywordFrom:
        return U"`from'";

      case Kind::KeywordIf:
        return U"`if'";

      case Kind::KeywordImport:
        return U"`import'";

      case Kind::KeywordNull:
        return U"`null'";

      case Kind::KeywordPass:
        return U"`pass'";

      case Kind::KeywordReturn:
        return U"`return'";

      case Kind::KeywordTrue:
        return U"`true'";

      case Kind::KeywordType:
        return U"`type'";

      case Kind::KeywordWhile:
        return U"`while'";

      case Kind::Dot:
        return U"`.'";

      case Kind::Comma:
        return U"`,'";

      case Kind::Colon:
        return U"`:'";

      case Kind::Semicolon:
        return U"`;'";

      case Kind::LeftParen:
        return U"`('";

      case Kind::RightParen:
        return U"`)'";

      case Kind::LeftBracket:
        return U"`['";

      case Kind::RightBracket:
        return U"`]'";

      case Kind::LeftBrace:
        return U"`{'";

      case Kind::RightBrace:
        return U"`}'";

      case Kind::Assign:
        return U"`='";

      case Kind::Add:
        return U"`+'";

      case Kind::Sub:
        return U"`-'";

      case Kind::Mul:
        return U"`*'";

      case Kind::Mod:
        return U"`%'";

      case Kind::Div:
        return U"`/'";

      case Kind::And:
        return U"`&'";

      case Kind::Or:
        return U"`|'";

      case Kind::Not:
        return U"`!'";

      case Kind::Eq:
        return U"`=='";

      case Kind::Ne:
        return U"`!='";

      case Kind::Lt:
        return U"`<'";

      case Kind::Gt:
        return U"`>'";

      case Kind::Lte:
        return U"`<='";

      case Kind::Gte:
        return U"`>='";

      case Kind::Arrow:
        return U"`->'";

      case Kind::Spread:
        return U"`...'";
    }

    return U"unknown token";
  }

  Token::Token(
    const ast::Position& position,
    Kind kind,
    const std::optional<std::u32string>& text
  )
    : m_position(position)
    , m_kind(kind)
    , m_text(text) {}

  Token::Token(const Token& that)
    : m_position(that.m_position)
    , m_kind(that.m_kind)
    , m_text(that.m_text) {}

  Token::Token(Token&& that)
    : m_position(std::move(that.m_position))
    , m_kind(std::move(that.m_kind))
    , m_text(std::move(that.m_text)) {}

  Token&
  Token::operator=(const Token& that)
  {
    m_position = that.m_position;
    m_kind = that.m_kind;
    m_text = that.m_text;

    return *this;
  }

  Token&
  Token::operator=(Token&& that)
  {
    m_position = std::move(that.m_position);
    m_kind = std::move(that.m_kind);
    m_text = std::move(that.m_text);

    return *this;
  }
}
