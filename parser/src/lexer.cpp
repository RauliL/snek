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
#include <peelo/unicode/ctype/isvalid.hpp>
#include <peelo/unicode/encoding/utf8.hpp>

#include "snek/parser/error.hpp"
#include "snek/parser/lexer.hpp"
#include "snek/parser/utils.hpp"

namespace snek::parser
{
  using keyword_map = std::unordered_map<std::u32string, TokenKind>;

  static const std::unordered_map<std::u32string, TokenKind> keywords =
  {
    { U"as", TokenKind::KeywordAs },
    { U"break", TokenKind::KeywordBreak },
    { U"const", TokenKind::KeywordConst },
    { U"continue", TokenKind::KeywordContinue },
    { U"else", TokenKind::KeywordElse },
    { U"export", TokenKind::KeywordExport },
    { U"false", TokenKind::KeywordFalse },
    { U"for", TokenKind::KeywordFor },
    { U"from", TokenKind::KeywordFrom },
    { U"if", TokenKind::KeywordIf },
    { U"import", TokenKind::KeywordImport },
    { U"let", TokenKind::KeywordLet },
    { U"null", TokenKind::KeywordNull },
    { U"pass", TokenKind::KeywordPass },
    { U"return", TokenKind::KeywordReturn },
    { U"true", TokenKind::KeywordTrue },
    { U"type", TokenKind::KeywordType },
    { U"while", TokenKind::KeywordWhile },
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
            throw SyntaxError{
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
              throw SyntaxError{
                position(),
                U"Unable to decode given input as UTF-8."
              };
          }
          for (std::size_t i = 1; i < length; ++i)
          {
            const auto c2 = m_input[m_offset++];

            if ((c2 & 0xc0) != 0x80)
            {
              throw SyntaxError{
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
          return Token(m_input->position(), TokenKind::Eof);
        }
        m_token_queue.push_back(Token(
          m_input->position(),
          TokenKind::NewLine
        ));
        do
        {
          m_indent_stack.pop();
          m_token_queue.push_back(Token(
            m_input->position(),
            TokenKind::Dedent
          ));
        }
        while (!m_indent_stack.empty());
      }
    }
  }

  void
  Lexer::ReadToken(TokenKind expected)
  {
    const auto token = ReadToken();

    if (!token.Is(expected))
    {
      throw SyntaxError{
        token.position,
        U"Unexpected " +
        token.ToString() +
        U"; Missing " +
        ToString(expected)
        + U'.'
      };
    }
  }

  void
  Lexer::UnreadToken(const Token& token)
  {
    if (!token.Is(TokenKind::Eof))
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
          return Token(m_input->position(), TokenKind::Eof);
        }
        LexLogicalLine();
      } else {
        return m_token_queue.front();
      }
    }
  }

  bool
  Lexer::PeekToken(TokenKind expected)
  {
    const auto token = ReadToken();

    UnreadToken(token);

    return token.Is(expected);
  }

  bool
  Lexer::PeekToken(std::initializer_list<TokenKind> expected)
  {
    const auto token = ReadToken();

    UnreadToken(token);

    return std::find_if(
      std::begin(expected),
      std::end(expected),
      [token](TokenKind kind)
      {
        return token.Is(kind);
      }
    ) != std::end(expected);
  }

  bool
  Lexer::PeekNextButOneToken(TokenKind expected)
  {
    const auto skip = ReadToken();
    const auto token = ReadToken();

    m_token_queue.push_front(token);
    m_token_queue.push_front(skip);

    return token.Is(expected);
  }

  bool
  Lexer::PeekReadToken(TokenKind expected)
  {
    const auto token = ReadToken();

    if (token.kind != expected)
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

    if (token.kind != TokenKind::Id)
    {
      throw SyntaxError{
        token.position,
        U"Unexpected " +
        ToString(token.kind) +
        U"; Missing identifier."
      };
    }

    return *token.text;
  }

  std::u32string
  Lexer::ReadString()
  {
    const auto token = ReadToken();

    if (token.kind != TokenKind::String)
    {
      throw SyntaxError{
        token.position,
        U"Unexpected " +
        ToString(token.kind) +
        U"; Missing string."
      };
    }

    return *token.text;
  }

  std::u32string
  Lexer::ReadRecordKey()
  {
    return PeekToken(TokenKind::String) ? *ReadToken().text : ReadId();
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
        m_token_queue.push_back(Token(position, TokenKind::Indent));
      }
    } else {
      const auto previous_indent = m_indent_stack.top();

      if (previous_indent > indent)
      {
        do
        {
          m_indent_stack.pop();
          m_token_queue.push_back(Token(position, TokenKind::Dedent));
        }
        while (!m_indent_stack.empty() && m_indent_stack.top() > indent);
      }
      else if (previous_indent < indent)
      {
        m_indent_stack.push(indent);
        m_token_queue.push_back(Token(position, TokenKind::Indent));
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
          TokenKind::NewLine
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
            ? TokenKind::LeftParen
            : c == '['
            ? TokenKind::LeftBracket
            : TokenKind::LeftBrace
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
            ? TokenKind::RightParen
            : c == ']'
            ? TokenKind::RightBracket
            : TokenKind::RightBrace
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
    TokenKind kind;

    switch (c)
    {
      case '.':
        if (m_input->PeekRead(U'.'))
        {
          if (!m_input->PeekRead(U'.'))
          {
            throw SyntaxError{ position, U"Unexpected `..'." };
          }
          kind = TokenKind::Spread;
        } else {
          kind = TokenKind::Dot;
        }
        break;

      case ',':
        kind = TokenKind::Comma;
        break;

      case ':':
        kind = TokenKind::Colon;
        break;

      case ';':
        kind = TokenKind::Semicolon;
        break;

      case '!':
        kind =
          m_input->PeekRead(U'=')
            ? TokenKind::NotEqual
            : TokenKind::Not;
        break;

      case '~':
        kind = TokenKind::BitwiseNot;
        break;

      case '^':
        kind =
          m_input->PeekRead(U'=')
            ? TokenKind::AssignBitwiseXor
            : TokenKind::BitwiseXor;
        break;

      case '=':
        kind =
          m_input->PeekRead(U'=')
            ? TokenKind::Equal
            : m_input->PeekRead(U'>')
            ? TokenKind::FatArrow
            : TokenKind::Assign;
        break;

      case '+':
        kind =
          m_input->PeekRead(U'=')
            ? TokenKind::AssignAdd
            : m_input->PeekRead(U'+')
            ? TokenKind::Increment
            : TokenKind::Add;
        break;

      case '-':
        kind =
          m_input->PeekRead(U'=')
            ? TokenKind::AssignSub
            : m_input->PeekRead(U'>')
            ? TokenKind::Arrow
            : m_input->PeekRead(U'-')
            ? TokenKind::Decrement
            : TokenKind::Sub;
        break;

      case '*':
        kind =
          m_input->PeekRead(U'=')
            ? TokenKind::AssignMul
            : TokenKind::Mul;
        break;

      case '/':
        kind =
          m_input->PeekRead(U'=')
            ? TokenKind::AssignDiv
            : TokenKind::Div;
        break;

      case '%':
        kind =
          m_input->PeekRead(U'=')
            ? TokenKind::AssignDiv
            : TokenKind::Mod;
        break;

      case '&':
        if (m_input->PeekRead(U'&'))
        {
          if (m_input->PeekRead(U'='))
          {
            kind = TokenKind::AssignLogicalAnd;
          } else {
            kind = TokenKind::LogicalAnd;
          }
        }
        else if (m_input->PeekRead(U'='))
        {
          kind = TokenKind::AssignBitwiseAnd;
        } else {
          kind = TokenKind::BitwiseAnd;
        }
        break;

      case '|':
        if (m_input->PeekRead(U'|'))
        {
          if (m_input->PeekRead(U'='))
          {
            kind = TokenKind::AssignLogicalOr;
          } else {
            kind = TokenKind::LogicalOr;
          }
        }
        else if (m_input->PeekRead(U'='))
        {
          kind = TokenKind::AssignBitwiseOr;
        } else {
          kind = TokenKind::BitwiseOr;
        }
        break;

      case '<':
        if (m_input->PeekRead(U'<'))
        {
          kind =
            m_input->PeekRead(U'=')
              ? TokenKind::AssignLeftShift
              : TokenKind::LeftShift;
        } else {
          kind =
            m_input->PeekRead(U'=')
              ? TokenKind::LessThanEqual
              : TokenKind::LessThan;
        }
        break;

      case '>':
        if (m_input->PeekRead(U'>'))
        {
          kind =
            m_input->PeekRead(U'=')
              ? TokenKind::AssignRightShift
              : TokenKind::RightShift;
        } else {
          kind =
            m_input->PeekRead(U'=')
              ? TokenKind::GreaterThanEqual
              : TokenKind::GreaterThan;
        }
        break;

      case '?':
        if (m_input->PeekRead(U'.'))
        {
          kind = TokenKind::ConditionalDot;
        }
        else if (m_input->PeekRead(U'?'))
        {
          if (m_input->PeekRead(U'='))
          {
            kind = TokenKind::AssignNullCoalescing;
          } else {
            kind = TokenKind::NullCoalescing;
          }
        } else {
          kind = TokenKind::Ternary;
        }
        break;

      default:
        throw SyntaxError{ position, U"Unexpected input." };
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

    return Token(position, TokenKind::Id, result);
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
        throw SyntaxError{
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

    return Token(position, TokenKind::String, result);
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
    TokenKind kind = TokenKind::Int;

    // TODO: Add support for different bases, such as 0x00 and so on.
    EatDigits(m_input, result);

    // Is it a decimal number?
    if (m_input->PeekRead(U'.'))
    {
      // TODO: Fix lexing stuff like `5.round()`.
      if (!m_input->Eof() && std::isdigit(m_input->Peek()))
      {
        kind = TokenKind::Float;
        result.append(1, U'.');
        EatDigits(m_input, result);
      } else {
        m_input->Unread(U'.');
      }
    }

    // Do we have an exponent?
    if (m_input->PeekRead(U'e') || m_input->PeekRead(U'E'))
    {
      kind = TokenKind::Float;
      result.append(1, 'e');
      if (m_input->Peek('+') || m_input->Peek('-'))
      {
        result.append(1, m_input->Read());
      }
      if (m_input->Eof() || !std::isdigit(m_input->Peek()))
      {
        throw SyntaxError{ position, U"Missing digits after `e'." };
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
      throw SyntaxError{
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
            throw SyntaxError{
              m_input->position(),
              U"Unterminated escape sequence."
            };
          }

          operand = m_input->Read();

          if (!std::isxdigit(operand))
          {
            throw SyntaxError{
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
          throw SyntaxError{
            m_input->position(),
            U"Illegal Unicode hex escape sequence."
          };
        }

        return c;
    }

    throw SyntaxError{
      m_input->position(),
      U"Illegal escape sequence in string literal."
    };
  }
}
