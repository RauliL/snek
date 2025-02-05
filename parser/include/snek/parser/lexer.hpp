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

#include <deque>
#include <stack>

#include "snek/parser/token.hpp"

namespace snek::parser
{
  class Lexer final
  {
  public:
    DEFAULT_COPY_AND_ASSIGN(Lexer);

    using iterator = std::string::const_iterator;

    Lexer(
      const iterator& begin,
      const iterator& end,
      const std::u32string& filename = U"<eval>",
      int line = 1,
      int column = 1
    )
      : m_current(begin)
      , m_end(end)
      , m_position{ filename, line, column } {}

    std::optional<Position> position() const;

    Token ReadToken();

    void ReadToken(Token::Kind expected);

    void UnreadToken(const Token& token);

    Token PeekToken();

    bool PeekToken(Token::Kind expected);

    bool PeekNextButOneToken(Token::Kind expected);

    bool PeekReadToken(Token::Kind expected);

    std::u32string ReadId();

    std::u32string ReadString();

  private:
    void LexLogicalLine();

    Token LexOperator();

    Token LexId();

    Token LexString();

    Token LexNumber();

    char32_t LexEscapeSequence();

    inline bool HasMoreChars() const
    {
      return m_current < m_end;
    }

    char32_t ReadChar();

    bool PeekChar(char expected) const;

    bool PeekReadChar(char expected);

  private:
    iterator m_current;
    iterator m_end;
    Position m_position;
    std::deque<Token> m_token_queue;
    std::stack<int> m_indent_stack;
  };
}
