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
#include <unordered_map>

#include <peelo/unicode/ctype/isvalid.hpp>

#include "snek/error.hpp"
#include "snek/parser/lexer.hpp"
#include "snek/parser/utils.hpp"

namespace snek::parser
{
  using keyword_map = std::unordered_map<std::u32string, Token::Kind>;

  static const std::unordered_map<std::u32string, Token::Kind> keywords =
  {
    { U"as", Token::Kind::KeywordAs },
    { U"break", Token::Kind::KeywordBreak },
    { U"const", Token::Kind::KeywordConst },
    { U"continue", Token::Kind::KeywordContinue },
    { U"else", Token::Kind::KeywordElse },
    { U"export", Token::Kind::KeywordExport },
    { U"false", Token::Kind::KeywordFalse },
    { U"if", Token::Kind::KeywordIf },
    { U"import", Token::Kind::KeywordImport },
    { U"let", Token::Kind::KeywordLet },
    { U"null", Token::Kind::KeywordNull },
    { U"pass", Token::Kind::KeywordPass },
    { U"return", Token::Kind::KeywordReturn },
    { U"true", Token::Kind::KeywordTrue },
    { U"type", Token::Kind::KeywordType },
    { U"while", Token::Kind::KeywordWhile },
  };

  const Position&
  Lexer::position() const
  {
    return m_token_queue.empty()
      ? m_position
      : m_token_queue.front().position();
  }

  Token
  Lexer::ReadToken()
  {
    for (;;)
    {
      if (!m_token_queue.empty())
      {
        const auto token = m_token_queue.front();

        m_token_queue.pop_front();

        return token;
      }
      if (HasMoreChars())
      {
        LexLogicalLine();
      } else {
        if (m_indent_stack.empty())
        {
          return Token(m_position, Token::Kind::Eof);
        }
        m_token_queue.push_back(Token(m_position, Token::Kind::NewLine));
        do
        {
          m_indent_stack.pop();
          m_token_queue.push_back(Token(m_position, Token::Kind::Dedent));
        }
        while (!m_indent_stack.empty());
      }
    }
  }

  void
  Lexer::ReadToken(Token::Kind expected)
  {
    const auto token = ReadToken();

    if (token.kind() != expected)
    {
      throw Error{
        token.position(),
        U"Unexpected " +
        token.ToString() +
        U"; Missing " +
        Token::ToString(expected)
        + U'.'
      };
    }
  }

  void
  Lexer::UnreadToken(const Token& token)
  {
    if (token.kind() != Token::Kind::Eof)
    {
      m_token_queue.push_front(token);
    }
  }

  Token
  Lexer::PeekToken()
  {
    for (;;)
    {
      if (m_token_queue.empty())
      {
        if (!HasMoreChars())
        {
          return Token(m_position, Token::Kind::Eof);
        }
        LexLogicalLine();
      } else {
        return m_token_queue.front();
      }
    }
  }

  bool
  Lexer::PeekToken(Token::Kind expected)
  {
    const auto token = ReadToken();

    UnreadToken(token);

    return token.kind() == expected;
  }

  bool
  Lexer::PeekNextButOneToken(Token::Kind expected)
  {
    const auto skip = ReadToken();
    const auto token = ReadToken();

    m_token_queue.push_front(token);
    m_token_queue.push_front(skip);

    return token.kind() == expected;
  }

  bool
  Lexer::PeekReadToken(Token::Kind expected)
  {
    const auto token = ReadToken();

    if (token.kind() != expected)
    {
      UnreadToken(token);

      return false;
    }

    return true;
  }

  std::u32string
  Lexer::ReadId()
  {
    const auto token = ReadToken();

    if (token.kind() != Token::Kind::Id)
    {
      throw Error{
        token.position(),
        U"Unexpected " +
        Token::ToString(token.kind()) +
        U"; Missing identifier."
      };
    }

    return *token.text();
  }

  void
  Lexer::LexLogicalLine()
  {
    const auto position = m_position;
    int indent = 0;
    int separator_count = 0;

    // Parse indentation at beginning of the line.
    while (PeekChar(' ') || PeekChar('\t'))
    {
      indent += ReadChar() == '\t' ? 8 : 1;
    }

    // If there is an comment after the initial indentation, skip that and call
    // it a day.
    if (PeekReadChar('#'))
    {
      while (HasMoreChars())
      {
        const auto c = ReadChar();

        if (c == '\n')
        {
          break;
        }
      }
      return;
    }

    // If it's an empty line, then do nothing else.
    if (!HasMoreChars() || PeekReadChar('\n'))
    {
      return;
    }

    // Then check if the indentation has changed from previous line.
    if (m_indent_stack.empty())
    {
      if (indent > 0)
      {
        m_indent_stack.push(indent);
        m_token_queue.push_back(Token(position, Token::Kind::Indent));
      }
    } else {
      const auto previous_indent = m_indent_stack.top();

      if (previous_indent > indent)
      {
        do
        {
          m_indent_stack.pop();
          m_token_queue.push_back(Token(position, Token::Kind::Dedent));
        }
        while (!m_indent_stack.empty() && m_indent_stack.top() > indent);
      }
      else if (previous_indent < indent)
      {
        m_indent_stack.push(indent);
        m_token_queue.push_back(Token(position, Token::Kind::Indent));
      }
    }

    // Lex tokens after initial indent.
    for (;;)
    {
      // End of input.
      if (!HasMoreChars())
      {
        break;
      }

      // End of line.
      if (PeekReadChar('\n') && !separator_count)
      {
        m_token_queue.push_back(Token(
          {
            m_position.filename,
            m_position.line,
            m_position.column - 1
          },
          Token::Kind::NewLine
        ));
        break;
      }

      // Skip whitespace before the next token.
      if (std::isspace(*m_current))
      {
        ReadChar();
        continue;
      }

      // Skip comments.
      if (PeekReadChar('#'))
      {
        while (!HasMoreChars())
        {
          const auto c = ReadChar();

          if (c == '\n')
          {
            break;
          }
        }
        break;
      }

      // Separators.
      if (PeekChar('(') || PeekChar('[') || PeekChar('{'))
      {
        const auto token_position = m_position;
        const auto c = ReadChar();

        ++separator_count;
        m_token_queue.push_back(Token(
          token_position,
          c == '('
            ? Token::Kind::LeftParen
            : c == '['
            ? Token::Kind::LeftBracket
            : Token::Kind::LeftBrace
        ));
        continue;
      }

      if (PeekChar(')') || PeekChar(']') || PeekChar('}'))
      {
        const auto token_position = m_position;
        const auto c = ReadChar();

        if (separator_count > 0)
        {
          --separator_count;
        }
        m_token_queue.push_back(Token(
          token_position,
          c == ')'
            ? Token::Kind::RightParen
            : c == ']'
            ? Token::Kind::RightBracket
            : Token::Kind::RightBrace
        ));
        continue;
      }

      // Is it identifier?
      if (utils::IsIdStart(*m_current))
      {
        m_token_queue.push_back(LexId());
        continue;
      }

      // Is it string literal?
      if (PeekChar('"') || PeekChar('\''))
      {
        m_token_queue.push_back(LexString());
        continue;
      }

      // Is it number literal?
      if (std::isdigit(*m_current))
      {
        m_token_queue.push_back(LexNumber());
        continue;
      }

      // Otherwise it must be an operator.
      m_token_queue.push_back(LexOperator());
    }
  }

  Token
  Lexer::LexOperator()
  {
    const auto position = m_position;
    const auto c = ReadChar();
    Token::Kind kind;

    switch (c)
    {
      case '.':
        if (PeekReadChar(U'.'))
        {
          if (!PeekReadChar(U'.'))
          {
            throw Error{ position, U"Unexpected `..'." };
          }
          kind = Token::Kind::Spread;
        } else {
          kind = Token::Kind::Dot;
        }
        break;

      case ',':
        kind = Token::Kind::Comma;
        break;

      case ':':
        kind = Token::Kind::Colon;
        break;

      case ';':
        kind = Token::Kind::Semicolon;
        break;

      case '!':
        kind = PeekReadChar(U'=') ? Token::Kind::NotEqual : Token::Kind::Not;
        break;

      case '~':
        kind = Token::Kind::BitwiseNot;
        break;

      case '^':
        kind =
          PeekReadChar(U'=')
            ? Token::Kind::AssignBitwiseXor
            : Token::Kind::BitwiseXor;
        break;

      case '=':
        kind =
          PeekReadChar(U'=')
            ? Token::Kind::Equal
            : PeekReadChar(U'>')
            ? Token::Kind::FatArrow
            : Token::Kind::Assign;
        break;

      case '+':
        kind =
          PeekReadChar(U'=')
            ? Token::Kind::AssignAdd
            : Token::Kind::Add;
        break;

      case '-':
        kind =
          PeekReadChar(U'=')
            ? Token::Kind::AssignSub
            : PeekReadChar(U'>')
            ? Token::Kind::Arrow
            : Token::Kind::Sub;
        break;

      case '*':
        kind =
          PeekReadChar(U'=')
            ? Token::Kind::AssignMul
            : Token::Kind::Mul;
        break;

      case '/':
        kind =
          PeekReadChar(U'=')
            ? Token::Kind::AssignDiv
            : Token::Kind::Div;
        break;

      case '%':
        kind =
          PeekReadChar(U'=')
            ? Token::Kind::AssignDiv
            : Token::Kind::Mod;
        break;

      case '&':
        kind =
          PeekReadChar(U'&')
            ? Token::Kind::LogicalAnd
            : PeekReadChar(U'=')
            ? Token::Kind::AssignBitwiseAnd
            : Token::Kind::BitwiseAnd;
        break;

      case '|':
        kind =
          PeekReadChar(U'|')
            ? Token::Kind::LogicalOr
            : PeekReadChar(U'=')
            ? Token::Kind::AssignBitwiseOr
            : Token::Kind::BitwiseOr;
        break;

      case '<':
        if (PeekReadChar(U'<'))
        {
          kind =
            PeekReadChar(U'=')
              ? Token::Kind::AssignLeftShift
              : Token::Kind::LeftShift;
        } else {
          kind =
            PeekReadChar(U'=')
              ? Token::Kind::LessThanEqual
              : Token::Kind::LessThan;
        }
        break;

      case '>':
        if (PeekReadChar(U'>'))
        {
          kind =
            PeekReadChar(U'=')
              ? Token::Kind::AssignRightShift
              : Token::Kind::RightShift;
        } else {
          kind =
            PeekReadChar(U'=')
              ? Token::Kind::GreaterThanEqual
              : Token::Kind::GreaterThan;
        }
        break;

      case '?':
        kind =
          PeekReadChar(U'.')
            ? Token::Kind::ConditionalDot
            : Token::Kind::Ternary;
        break;

      default:
        throw Error{ position, U"Unexpected input." };
    }

    return Token(position, kind);
  }

  Token
  Lexer::LexId()
  {
    const auto position = m_position;
    std::u32string result;
    keyword_map::const_iterator keyword_index;

    do
    {
      result.append(1, ReadChar());
    }
    while (HasMoreChars() && utils::IsIdPart(*m_current)); // TODO: Fix Unicode

    if ((keyword_index = keywords.find(result)) != std::end(keywords))
    {
      return Token(position, keyword_index->second);
    }

    return Token(position, Token::Kind::Id, result);
  }

  Token
  Lexer::LexString()
  {
    const auto position = m_position;
    const auto separator = ReadChar();
    std::u32string result;
    char32_t c;

    for (;;)
    {
      if (!HasMoreChars())
      {
        throw Error{
          position,
          std::u32string(U"Unterminated string literal; Missing `") +
          separator +
          U"'."
        };
      }
      c = ReadChar();
      if (c == separator)
      {
        break;
      }
      else if (c == '\\')
      {
        result.append(1, LexEscapeSequence());
      } else {
        result.append(1, c);
      }
    }

    return Token(position, Token::Kind::String, result);
  }

  Token
  Lexer::LexNumber()
  {
    const auto position = m_position;
    std::u32string result;
    Token::Kind kind = Token::Kind::Int;

    // TODO: Add support for different bases, such as 0x00 and so on.
    do
    {
      const auto c = ReadChar();

      if (c != '_')
      {
        result.append(1, c);
      }
    }
    while (HasMoreChars() && utils::IsNumberPart(*m_current));

    // Is it a decimal number?
    if (
      PeekChar(U'.') &&
      m_current + 1 < m_end &&
      std::isdigit(*(m_current + 1))
    )
    {
      kind = Token::Kind::Float;
      result.append(1, ReadChar());
      do
      {
        const auto c = ReadChar();

        if (c != '_')
        {
          result.append(1, c);
        }
      }
      while (HasMoreChars() && utils::IsNumberPart(*m_current));
    }

    // Do we have an exponent?
    if (PeekReadChar(U'e') || PeekReadChar(U'E'))
    {
      kind = Token::Kind::Float;
      result.append(1, 'e');
      if (PeekChar('+') || PeekChar('-'))
      {
        result.append(1, ReadChar());
      }
      if (!HasMoreChars() || !std::isdigit(*m_current))
      {
        throw Error{ position, U"Missing digits after `e'." };
      }
      do
      {
        const auto c = ReadChar();

        if (c != '_')
        {
          result.append(1, c);
        }
      }
      while (HasMoreChars() && utils::IsNumberPart(*m_current));
    }

    return Token(position, kind, result);
  }

  char32_t
  Lexer::LexEscapeSequence()
  {
    using peelo::unicode::ctype::isvalid;

    if (!HasMoreChars())
    {
      throw Error{
        m_position,
        U"Unexpected end of input; Missing escape sequence."
      };
    }

    switch (ReadChar())
    {
      case 'b':
        return 010;

      case 't':
        return 011;

      case 'n':
        return 012;

      case 'f':
        return 014;

      case 'r':
        return 015;

      case '"':
      case '\'':
      case '\\':
      case '/':
        return *(m_current - 1);

      case 'u':
        {
          char32_t c = 0;

          for (int i = 0; i < 4; ++i)
          {
            if (!HasMoreChars())
            {
              throw Error{
                m_position,
                U"Unterminated escape sequence."
              };
            }
            else if (!std::isxdigit(*m_current))
            {
              throw Error{
                m_position,
                U"Illegal Unicode hex escape sequence."
              };
            }

            if (*m_current >= 'A' && *m_current <= 'F')
            {
              c = c * 16 + (ReadChar() - 'A' + 10);
            }
            else if (*m_current >= 'a' && *m_current <= 'f')
            {
              c = c * 16 + (ReadChar() - 'a' + 10);
            } else {
              c = c * 16 + (ReadChar() - '0');
            }
          }

          if (!isvalid(c))
          {
            throw Error{
              m_position,
              U"Illegal Unicode hex escape sequence."
            };
          }

          return c;
        }
        break;
    }

    throw Error{
      m_position,
      U"Illegal escape sequence in string literal."
    };
  }

  char32_t
  Lexer::ReadChar()
  {
    const auto c = *m_current++;

    if (utils::IsNewLine(c))
    {
      if (c == '\r' && m_current < m_end && *m_current == '\n')
      {
        ++m_current;
      }
      ++m_position.line;
      m_position.column = 1;

      return '\n';
    }
    ++m_position.column;
    // TODO: Decode UTF-8.

    return c;
  }

  bool
  Lexer::PeekChar(char expected) const
  {
    return m_current < m_end && *m_current == expected;
  }

  bool
  Lexer::PeekReadChar(char expected)
  {
    if (PeekChar(expected))
    {
      ReadChar();

      return true;
    }

    return false;
  }
}
