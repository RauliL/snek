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
#include <peelo/unicode/encoding/utf8.hpp>

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
    { U"for", Token::Kind::KeywordFor },
    { U"from", Token::Kind::KeywordFrom },
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

  char32_t
  Lexer::Input::Read()
  {
    char32_t c;

    if (m_char_queue.empty())
    {
      c = Advance();
      if (utils::IsNewLine(c))
      {
        if (c == '\r')
        {
          const auto c2 = HasMoreInput() ? Advance() : '\n';

          if (c2 != '\n')
          {
            m_char_queue.push_back(c2);
          }
        }
        ++m_position.line;
        m_position.column = 1;
        c = '\n';
      } else {
        ++m_position.column;
      }
    } else {
      c = m_char_queue.front();
      m_char_queue.pop_front();
    }

    return c;
  }

  char32_t
  Lexer::Input::Peek()
  {
    if (m_char_queue.empty())
    {
      const auto c = Advance();

      m_char_queue.push_back(c);

      return c;
    }

    return m_char_queue.front();
  }

  char32_t
  Lexer::Input::PeekNextButOne()
  {
    const auto size = m_char_queue.size();

    if (size > 1)
    {
      return m_char_queue[1];
    }
    else if (size == 1)
    {
      if (HasMoreInput())
      {
        const auto c = Advance();

        m_char_queue.push_back(c);

        return c;
      }
    }
    else if (HasMoreInput())
    {
      m_char_queue.push_back(Advance());
      if (HasMoreInput())
      {
        const auto c = Advance();

        m_char_queue.push_back(c);

        return c;
      }
    }

    return 0;
  }

  namespace
  {
    class Utf8Input final : public Lexer::Input
    {
    public:
      explicit Utf8Input(const Position& position, const std::string& input)
        : Input(position)
        , m_input(input)
        , m_offset(0) {}

    protected:
      inline bool HasMoreInput() const override
      {
        return m_offset < m_input.length();
      }

      char32_t Advance() override
      {
        using peelo::unicode::encoding::utf8::sequence_length;

        const auto c = m_input[m_offset++];
        const auto length = sequence_length(c);

        if (length > 1)
        {
          char32_t result;

          if (m_offset + (length - 1) >= m_input.length())
          {
            throw Error{
              position(),
              U"Unable to decode given input as UTF-8."
            };
          }
          switch (length)
          {
            case 2:
              result = static_cast<char32_t>(c & 0x1f);
              break;

            case 3:
              result = static_cast<char32_t>(c & 0x0f);
              break;

            case 4:
              result = static_cast<char32_t>(c & 0x07);
              break;

            default:
              throw Error{
                position(),
                U"Unable to decode given input as UTF-8."
              };
          }
          for (std::size_t i = 1; i < length; ++i)
          {
            const auto c2 = m_input[m_offset++];

            if ((c2 & 0xc0) != 0x80)
            {
              throw Error{
                position(),
                U"Unable to decode given input as UTF-8."
              };
            }
            result = (result << 6) | (c2 & 0x3f);
          }

          return result;
        }

        return static_cast<char32_t>(c);
      }

    private:
      const std::string m_input;
      std::string::size_type m_offset;
    };

    class UnicodeInput final : public Lexer::Input
    {
    public:
      explicit UnicodeInput(
        const Position& position,
        const std::u32string& input
      )
        : Input(position)
        , m_input(input)
        , m_offset(0) {}

    protected:
      inline bool HasMoreInput() const override
      {
        return m_offset < m_input.length();
      }

      inline char32_t Advance() override
      {
        return m_input[m_offset++];
      }

    private:
      const std::u32string m_input;
      std::string::size_type m_offset;
    };
  }

  Lexer::Lexer(
    const std::string& source,
    const std::u32string& filename,
    int line,
    int column
  )
    : m_input(std::make_shared<Utf8Input>(
        Position{ filename, line, column },
        source
      )) {}

  Lexer::Lexer(
    const std::u32string& source,
    const std::u32string& filename,
    int line,
    int column
  )
    : m_input(std::make_shared<UnicodeInput>(
        Position{ filename, line, column },
        source
      )) {}

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
      if (!m_input->Eof())
      {
        LexLogicalLine();
      } else {
        if (m_indent_stack.empty())
        {
          return Token(m_input->position(), Token::Kind::Eof);
        }
        m_token_queue.push_back(Token(
          m_input->position(),
          Token::Kind::NewLine
        ));
        do
        {
          m_indent_stack.pop();
          m_token_queue.push_back(Token(
            m_input->position(),
            Token::Kind::Dedent
          ));
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
        if (m_input->Eof())
        {
          return Token(m_input->position(), Token::Kind::Eof);
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

  std::u32string
  Lexer::ReadString()
  {
    const auto token = ReadToken();

    if (token.kind() != Token::Kind::String)
    {
      throw Error{
        token.position(),
        U"Unexpected " +
        Token::ToString(token.kind()) +
        U"; Missing string."
      };
    }

    return *token.text();
  }

  void
  Lexer::LexLogicalLine()
  {
    const auto position = m_input->position();
    int indent = 0;
    int separator_count = 0;

    // Parse indentation at beginning of the line.
    while (m_input->Peek(' ') || m_input->Peek('\t'))
    {
      indent += m_input->Read() == '\t' ? 8 : 1;
    }

    // If there is an comment after the initial indentation, skip that and call
    // it a day.
    if (m_input->PeekRead('#'))
    {
      while (!m_input->Eof())
      {
        const auto c = m_input->Read();

        if (c == '\n')
        {
          break;
        }
      }
      return;
    }

    // If it's an empty line, then do nothing else.
    if (m_input->Eof() || m_input->PeekRead('\n'))
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
      if (m_input->Eof())
      {
        break;
      }

      // End of line.
      if (m_input->PeekRead('\n') && !separator_count)
      {
        m_token_queue.push_back(Token(
          std::make_optional(m_input->position()),
          Token::Kind::NewLine
        ));
        break;
      }

      // Skip whitespace before the next token.
      if (std::isspace(m_input->Peek()))
      {
        m_input->Read();
        continue;
      }

      // Skip comments.
      if (m_input->PeekRead('#'))
      {
        while (!m_input->Eof())
        {
          const auto c = m_input->Read();

          if (c == '\n')
          {
            break;
          }
        }
        break;
      }

      // Separators.
      if (m_input->Peek('(') || m_input->Peek('[') || m_input->Peek('{'))
      {
        const auto token_position = m_input->position();
        const auto c = m_input->Read();

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

      if (m_input->Peek(')') || m_input->Peek(']') || m_input->Peek('}'))
      {
        const auto token_position = m_input->position();
        const auto c = m_input->Read();

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
      if (utils::IsIdStart(m_input->Peek()))
      {
        m_token_queue.push_back(LexId());
        continue;
      }

      // Is it string literal?
      if (m_input->Peek('"') || m_input->Peek('\''))
      {
        m_token_queue.push_back(LexString());
        continue;
      }

      // Is it number literal?
      if (std::isdigit(m_input->Peek()))
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
    const auto position = m_input->position();
    const auto c = m_input->Read();
    Token::Kind kind;

    switch (c)
    {
      case '.':
        if (m_input->PeekRead(U'.'))
        {
          if (!m_input->PeekRead(U'.'))
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
        kind =
          m_input->PeekRead(U'=')
            ? Token::Kind::NotEqual
            : Token::Kind::Not;
        break;

      case '~':
        kind = Token::Kind::BitwiseNot;
        break;

      case '^':
        kind =
          m_input->PeekRead(U'=')
            ? Token::Kind::AssignBitwiseXor
            : Token::Kind::BitwiseXor;
        break;

      case '=':
        kind =
          m_input->PeekRead(U'=')
            ? Token::Kind::Equal
            : m_input->PeekRead(U'>')
            ? Token::Kind::FatArrow
            : Token::Kind::Assign;
        break;

      case '+':
        kind =
          m_input->PeekRead(U'=')
            ? Token::Kind::AssignAdd
            : m_input->PeekRead(U'+')
            ? Token::Kind::Increment
            : Token::Kind::Add;
        break;

      case '-':
        kind =
          m_input->PeekRead(U'=')
            ? Token::Kind::AssignSub
            : m_input->PeekRead(U'>')
            ? Token::Kind::Arrow
            : m_input->PeekRead(U'-')
            ? Token::Kind::Decrement
            : Token::Kind::Sub;
        break;

      case '*':
        kind =
          m_input->PeekRead(U'=')
            ? Token::Kind::AssignMul
            : Token::Kind::Mul;
        break;

      case '/':
        kind =
          m_input->PeekRead(U'=')
            ? Token::Kind::AssignDiv
            : Token::Kind::Div;
        break;

      case '%':
        kind =
          m_input->PeekRead(U'=')
            ? Token::Kind::AssignDiv
            : Token::Kind::Mod;
        break;

      case '&':
        kind =
          m_input->PeekRead(U'&')
            ? Token::Kind::LogicalAnd
            : m_input->PeekRead(U'=')
            ? Token::Kind::AssignBitwiseAnd
            : Token::Kind::BitwiseAnd;
        break;

      case '|':
        kind =
          m_input->PeekRead(U'|')
            ? Token::Kind::LogicalOr
            : m_input->PeekRead(U'=')
            ? Token::Kind::AssignBitwiseOr
            : Token::Kind::BitwiseOr;
        break;

      case '<':
        if (m_input->PeekRead(U'<'))
        {
          kind =
            m_input->PeekRead(U'=')
              ? Token::Kind::AssignLeftShift
              : Token::Kind::LeftShift;
        } else {
          kind =
            m_input->PeekRead(U'=')
              ? Token::Kind::LessThanEqual
              : Token::Kind::LessThan;
        }
        break;

      case '>':
        if (m_input->PeekRead(U'>'))
        {
          kind =
            m_input->PeekRead(U'=')
              ? Token::Kind::AssignRightShift
              : Token::Kind::RightShift;
        } else {
          kind =
            m_input->PeekRead(U'=')
              ? Token::Kind::GreaterThanEqual
              : Token::Kind::GreaterThan;
        }
        break;

      case '?':
        kind =
          m_input->PeekRead(U'.')
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
    const auto position = m_input->position();
    std::u32string result;
    keyword_map::const_iterator keyword_index;

    do
    {
      result.append(1, m_input->Read());
    }
    while (!m_input->Eof() && utils::IsIdPart(m_input->Peek()));

    if ((keyword_index = keywords.find(result)) != std::end(keywords))
    {
      return Token(position, keyword_index->second);
    }

    return Token(position, Token::Kind::Id, result);
  }

  Token
  Lexer::LexString()
  {
    const auto position = m_input->position();
    const auto separator = m_input->Read();
    std::u32string result;
    char32_t c;

    for (;;)
    {
      if (m_input->Eof())
      {
        throw Error{
          position,
          std::u32string(U"Unterminated string literal; Missing `") +
          separator +
          U"'."
        };
      }
      c = m_input->Read();
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

  static inline void
  EatDigits(
    const std::shared_ptr<Lexer::Input>& input,
    std::u32string& result
  )
  {
    do
    {
      const auto c = input->Read();

      if (c != '_')
      {
        result.append(1, c);
      }
    }
    while (!input->Eof() && utils::IsNumberPart(input->Peek()));
  }

  Token
  Lexer::LexNumber()
  {
    const auto position = m_input->position();
    std::u32string result;
    Token::Kind kind = Token::Kind::Int;

    // TODO: Add support for different bases, such as 0x00 and so on.
    EatDigits(m_input, result);

    // Is it a decimal number?
    if (m_input->Peek('.') && std::isdigit(m_input->PeekNextButOne()))
    {
      kind = Token::Kind::Float;
      result.append(1, m_input->Read());
      EatDigits(m_input, result);
    }

    // Do we have an exponent?
    if (m_input->PeekRead(U'e') || m_input->PeekRead(U'E'))
    {
      kind = Token::Kind::Float;
      result.append(1, 'e');
      if (m_input->Peek('+') || m_input->Peek('-'))
      {
        result.append(1, m_input->Read());
      }
      if (m_input->Eof() || !std::isdigit(m_input->Peek()))
      {
        throw Error{ position, U"Missing digits after `e'." };
      }
      EatDigits(m_input, result);
    }

    return Token(position, kind, result);
  }

  char32_t
  Lexer::LexEscapeSequence()
  {
    using peelo::unicode::ctype::isvalid;

    char32_t c;

    if (m_input->Eof())
    {
      throw Error{
        m_input->position(),
        U"Unexpected end of input; Missing escape sequence."
      };
    }

    switch (c = m_input->Read())
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
        return c;

      case 'u':
        c = 0;

        for (int i = 0; i < 4; ++i)
        {
          char32_t operand;

          if (m_input->Eof())
          {
            throw Error{
              m_input->position(),
              U"Unterminated escape sequence."
            };
          }

          operand = m_input->Read();

          if (!std::isxdigit(operand))
          {
            throw Error{
              m_input->position(),
              U"Illegal Unicode hex escape sequence."
            };
          }

          if (operand >= 'A' && operand <= 'F')
          {
            c = c * 16 + (operand - 'A' + 10);
          }
          else if (operand >= 'a' && operand <= 'f')
          {
            c = c * 16 + (operand - 'a' + 10);
          } else {
            c = c * 16 + (operand - '0');
          }
        }

        if (!isvalid(c))
        {
          throw Error{
            m_input->position(),
            U"Illegal Unicode hex escape sequence."
          };
        }

        return c;
    }

    throw Error{
      m_input->position(),
      U"Illegal escape sequence in string literal."
    };
  }
}
