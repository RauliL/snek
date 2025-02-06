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
#include <memory>
#include <stack>

#include "snek/parser/token.hpp"

namespace snek::parser
{
  class Lexer final
  {
  public:
    DEFAULT_COPY_AND_ASSIGN(Lexer);

    class Input
    {
    public:
      DISALLOW_COPY_AND_ASSIGN(Input);

      explicit Input(const Position& position)
        : m_position(position) {}

      inline const Position& position() const
      {
        return m_position;
      }

      inline bool Eof() const
      {
        return m_char_queue.empty() && !HasMoreInput();
      }

      char32_t Read();

      inline void Unread(char32_t c)
      {
        m_char_queue.push_back(c);
      }

      char32_t Peek();

      inline bool Peek(char32_t expected)
      {
        return !Eof() && Peek() == expected;
      }

      inline bool PeekRead(char32_t expected)
      {
        if (Peek(expected))
        {
          Read();

          return true;
        }

        return false;
      }

    protected:
      virtual bool HasMoreInput() const = 0;

      virtual char32_t Advance() = 0;

    private:
      Position m_position;
      std::deque<char32_t> m_char_queue;
    };

    Lexer(
      const std::string& input,
      const std::u32string& filename = U"<eval>",
      int line = 1,
      int column = 1
    );

    Lexer(
      const std::u32string& input,
      const std::u32string& filename = U"<eval>",
      int line = 1,
      int column = 1
    );

    inline std::optional<Position> position() const
    {
      return m_token_queue.empty()
        ? m_input->position()
        : m_token_queue.front().position();
    }

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

  private:
    std::shared_ptr<Input> m_input;
    std::deque<Token> m_token_queue;
    std::stack<int> m_indent_stack;
  };
}
