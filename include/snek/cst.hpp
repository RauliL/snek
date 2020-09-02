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
#pragma once

#include <optional>

#include <peelo/unicode/ctype/isalnum.hpp>
#include <peelo/unicode/ctype/isalpha.hpp>

#include <snek/ast/position.hpp>

namespace snek::cst
{
  enum class Kind
  {
    // Separators
    EndOfInput,
    NewLine,
    Indent,
    Dedent,
    LeftParen,
    RightParen,
    LeftBracket,
    RightBracket,
    LeftBrace,
    RightBrace,
    Dot,
    Comma,
    Colon,
    Semicolon,
    Arrow,
    Spread,

    // Literals
    Id,
    Int,
    Float,
    Str,

    // Reserved keywords
    KeywordAs,
    KeywordBreak,
    KeywordContinue,
    KeywordElse,
    KeywordExport,
    KeywordFalse,
    KeywordFrom,
    KeywordIf,
    KeywordImport,
    KeywordNull,
    KeywordPass,
    KeywordReturn,
    KeywordTrue,
    KeywordType,
    KeywordWhile,

    // Operators
    Assign,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    And,
    Or,
    Not,
    Eq,
    Ne,
    Lt,
    Gt,
    Lte,
    Gte,
  };

  inline bool is_identifier_start(char32_t c)
  {
    return c == '$' || c == '_' || peelo::unicode::ctype::isalpha(c);
  }

  inline bool is_identifier_part(char32_t c)
  {
    return c == '$' || c == '_' || peelo::unicode::ctype::isalnum(c);
  }

  inline bool is_identifier(const std::u32string& text)
  {
    const auto length = text.length();

    if (!length || !is_identifier_start(text[0]))
    {
      return false;
    }
    for (std::size_t i = 1; i < length; ++i)
    {
      if (!is_identifier_part(text[i]))
      {
        return false;
      }
    }

    return true;
  }

  std::u32string stringify(const std::u32string& text);

  std::u32string to_string(Kind kind);

  class Token
  {
  public:
    explicit Token(
      const ast::Position& position,
      Kind kind,
      const std::optional<std::u32string>& text = std::nullopt
    );
    Token(const Token& that);
    Token(Token&& that);
    Token& operator=(const Token& that);
    Token& operator=(Token&& that);

    inline const ast::Position& position() const
    {
      return m_position;
    }

    inline Kind kind() const
    {
      return m_kind;
    }

    inline const std::optional<std::u32string>& text() const
    {
      return m_text;
    }

  private:
    ast::Position m_position;
    Kind m_kind;
    std::optional<std::u32string> m_text;
  };
}
