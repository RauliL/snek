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
#include <cctype>
#include <stack>
#include <unordered_map>

#include <peelo/unicode/ctype/isvalid.hpp>

#include <snek/cst.hpp>
#include <snek/lexer.hpp>

namespace snek::lexer
{
  using iterator = std::u32string::const_iterator;
  using keyword_container = std::unordered_map<std::u32string, cst::Kind>;
  using token_result_type = peelo::result<cst::Token, Error>;

  static const keyword_container reserved_keywords =
  {
    { U"as", cst::Kind::KeywordAs },
    { U"break", cst::Kind::KeywordBreak },
    { U"continue", cst::Kind::KeywordContinue },
    { U"else", cst::Kind::KeywordElse },
    { U"export", cst::Kind::KeywordExport },
    { U"false", cst::Kind::KeywordFalse },
    { U"from", cst::Kind::KeywordFrom },
    { U"if", cst::Kind::KeywordIf },
    { U"import", cst::Kind::KeywordImport },
    { U"null", cst::Kind::KeywordNull },
    { U"pass", cst::Kind::KeywordPass },
    { U"return", cst::Kind::KeywordReturn },
    { U"true", cst::Kind::KeywordTrue },
    { U"type", cst::Kind::KeywordType },
    { U"while", cst::Kind::KeywordWhile },
  };

  static inline bool
  is_new_line(char32_t c)
  {
    return c == '\n' || c == '\r';
  }

  static inline bool
  is_num_part(char32_t c)
  {
    return std::isdigit(c) || c == '_';
  }

  static inline token_result_type
  make_token(
    const ast::Position& position,
    cst::Kind kind,
    const std::optional<std::u32string>& text = std::nullopt
  )
  {
    return token_result_type::ok(cst::Token(position, kind, text));
  }

  namespace
  {
    struct State
    {
      iterator current;
      const iterator end;
      ast::Position position;

      inline bool eof() const
      {
        return current >= end;
      }

      inline bool peek(char32_t c, std::size_t offset = 0) const
      {
        return current + offset < end && *(current + offset) == c;
      }

      inline bool peek_read(char32_t c)
      {
        if (peek(c))
        {
          advance();

          return true;
        }

        return false;
      }

      char32_t advance()
      {
        const auto c = *current++;

        if (is_new_line(c))
        {
          if (c == '\r' && !eof() && *current == '\n')
          {
            ++current;
          }
          ++position.line;
          position.column = 1;

          return '\n';
        }
        ++position.column;

        return c;
      }
    };
  }

  static cst::Token lex_identifier(State&);
  static token_result_type lex_string_literal(State&);
  static token_result_type lex_number_literal(State&);
  static token_result_type lex_operator(State&);

  // TODO: Add support for explicit line joining with `\'
  static std::optional<Error>
  lex_logical_line(
    State& state,
    std::vector<cst::Token>& tokens,
    std::stack<int>& indent_stack
  )
  {
    const auto position = state.position;
    int indent = 0;
    int separator_count = 0;

    // Parse indentation at beginning of the line.
    while (state.peek(' ') || state.peek('\t'))
    {
      indent += state.advance() == '\t' ? 8 : 1;
    }

    // If there is an comment after the initial indentation, skip that and call
    // it a day.
    if (state.peek_read('#'))
    {
      while (!state.eof())
      {
        const auto c = state.advance();

        if (c == '\n')
        {
          break;
        }
      }

      return std::nullopt;
    }

    // If it's an empty line, then do nothing else.
    if (state.eof() || state.peek_read('\n'))
    {
      return std::nullopt;
    }

    // Then check if the indentation has changed from previous line.
    if (indent_stack.empty())
    {
      if (indent > 0)
      {
        indent_stack.push(indent);
        tokens.push_back(cst::Token(position, cst::Kind::Indent));
      }
    } else {
      auto previous_indent = indent_stack.top();

      if (previous_indent > indent)
      {
        do
        {
          if (indent_stack.empty())
          {
            break;
          }
          previous_indent = indent_stack.top();
          indent_stack.pop();
          tokens.push_back(cst::Token(position, cst::Kind::Dedent));
          if (previous_indent < indent)
          {
            return std::make_optional<Error>({
              position,
              U"Indentation mismatch."
            });
          }
        }
        while (previous_indent > indent);
      }
      else if (previous_indent < indent)
      {
        indent_stack.push(indent);
        tokens.push_back(cst::Token(position, cst::Kind::Indent));
      }
    }

    // Lex tokens after initial indent.
    for (;;)
    {
      // End of input.
      if (state.eof())
      {
        break;
      }

      // End of line.
      if (state.peek_read('\n') && !separator_count)
      {
        tokens.push_back(cst::Token(
          {
            state.position.file,
            state.position.line,
            state.position.column - 1
          },
          cst::Kind::NewLine
        ));
        break;
      }

      // Skip whitespace before the next token.
      if (std::isspace(*state.current))
      {
        state.advance();
        continue;
      }

      // Skip comments.
      if (state.peek_read('#'))
      {
        while (!state.eof())
        {
          const auto c = state.advance();

          if (c == '\n')
          {
            break;
          }
        }
        break;
      }

      // Separators.
      if (state.peek('(') || state.peek('[') || state.peek('{'))
      {
        const auto position = state.position;
        const auto c = state.advance();

        ++separator_count;
        tokens.push_back(cst::Token(
          position,
          c == '('
            ? cst::Kind::LeftParen
            : c == '['
            ? cst::Kind::LeftBracket
            : cst::Kind::LeftBrace
        ));
        continue;
      }

      if (state.peek(')') || state.peek(']') || state.peek('}'))
      {
        const auto position = state.position;
        const auto c = state.advance();

        if (separator_count > 0)
        {
          --separator_count;
        }
        tokens.push_back(cst::Token(
          position,
          c == ')'
            ? cst::Kind::RightParen
            : c == ']'
            ? cst::Kind::RightBracket
            : cst::Kind::RightBrace
        ));
        continue;
      }

      // Is it identifier?
      if (cst::is_identifier_start(*state.current))
      {
        tokens.push_back(lex_identifier(state));
        continue;
      }

      // Is it string literal?
      if (state.peek('"') || state.peek('\''))
      {
        const auto result = lex_string_literal(state);

        if (!result)
        {
          return result.error();
        }
        tokens.push_back(result.value());
        continue;
      }

      // TODO: Add support for binary literals.

      // Is it number literal?
      if (std::isdigit(*state.current))
      {
        const auto result = lex_number_literal(state);

        if (!result)
        {
          return result.error();
        }
        tokens.push_back(result.value());
        continue;
      }

      // Otherwise it must be an operator.
      const auto result = lex_operator(state);

      if (!result)
      {
        return result.error();
      }
      tokens.push_back(result.value());
    }

    return std::nullopt;
  }

  result_type
  lex(
    const std::u32string& source,
    const std::u32string& file,
    int line,
    int column
  )
  {
    State state = {
      std::begin(source),
      std::end(source),
      { file, line, column },
    };
    std::vector<cst::Token> tokens;
    std::stack<int> indent_stack;

    while (!state.eof())
    {
      if (const auto error = lex_logical_line(state, tokens, indent_stack))
      {
        return result_type::error(*error);
      }
    }

    if (!indent_stack.empty())
    {
      tokens.push_back(cst::Token(state.position, cst::Kind::NewLine));
      do
      {
        indent_stack.pop();
        tokens.push_back(cst::Token(state.position, cst::Kind::Dedent));
      }
      while (!indent_stack.empty());
    }

    return result_type::ok(tokens);
  }

  static cst::Token
  lex_identifier(State& state)
  {
    const auto position = state.position;
    std::u32string identifier;
    keyword_container::const_iterator keyword;

    identifier.append(1, state.advance());
    while (!state.eof() && cst::is_identifier_part(*state.current))
    {
      identifier.append(1, state.advance());
    }
    keyword = reserved_keywords.find(identifier);

    return cst::Token(
      position,
      keyword == std::end(reserved_keywords) ? cst::Kind::Id : keyword->second,
      identifier
    );
  }

  static std::optional<Error>
  lex_escape_sequence(State& state, std::u32string& buffer)
  {
    if (state.eof())
    {
      return std::make_optional<Error>({
        state.position,
        U"Unexpected end of input; Missing escape sequence."
      });
    }

    switch (state.advance())
    {
      case 'b':
        buffer.append(1, 010);
        break;

      case 't':
        buffer.append(1, 011);
        break;

      case 'n':
        buffer.append(1, 012);
        break;

      case 'f':
        buffer.append(1, 014);
        break;

      case 'r':
        buffer.append(1, 015);
        break;

      case '"':
      case '\'':
      case '\\':
      case '/':
        buffer.append(1, *(state.current - 1));
        break;

      case 'u':
        {
          char32_t result = 0;

          for (int i = 0; i < 4; ++i)
          {
            if (state.eof())
            {
              return std::make_optional<Error>({
                state.position,
                U"Unterminated escape sequence."
              });
            }
            else if (!std::isxdigit(*state.current))
            {
              return std::make_optional<Error>({
                state.position,
                U"Illegal Unicode hex escape sequence."
              });
            }

            if (*state.current >= 'A' && *state.current <= 'F')
            {
              result = result * 16 + (state.advance() - 'A' + 10);
            }
            else if (*state.current >= 'a' && *state.current <= 'f')
            {
              result = result * 16 + (state.advance() - 'a' + 10);
            } else {
              result = result * 16 + (state.advance() - '0');
            }
          }

          if (!peelo::unicode::ctype::isvalid(result))
          {
            return std::make_optional<Error>({
              state.position,
              U"Illegal Unicode hex escape sequence."
            });
          }

          buffer.append(1, result);
        }
        break;

      default:
        return std::make_optional<Error>({
          state.position,
          U"Illegal escape sequence in string literal."
        });
    }

    return std::nullopt;
  }

  static token_result_type
  lex_string_literal(State& state)
  {
    const auto position = state.position;
    const auto separator = state.advance();
    std::u32string buffer;
    char32_t c;

    for (;;)
    {
      if (state.eof())
      {
        return token_result_type::error({
          position,
          std::u32string(U"Unterminated string literal; Missing `") +
          separator +
          U"'."
        });
      }
      c = state.advance();
      if (c == separator)
      {
        break;
      }
      else if (c == '\\')
      {
        if (const auto error = lex_escape_sequence(state, buffer))
        {
          return token_result_type::error(*error);
        }
      } else {
        buffer.append(1, c);
      }
    }

    return token_result_type::ok(cst::Token(position, cst::Kind::Str, buffer));
  }

  static token_result_type
  lex_number_literal(State& state)
  {
    const auto position = state.position;
    std::u32string buffer;
    cst::Kind kind = cst::Kind::Int;

    // TODO: Add support for different bases, such as 0x00 and so on.
    do
    {
      const auto c = state.advance();

      if (c != '_')
      {
        buffer.append(1, c);
      }
    }
    while (!state.eof() && is_num_part(*state.current));
    if (state.peek_read('.'))
    {
      kind = cst::Kind::Float;
      buffer.append(1, '.');
      if (state.eof() || !std::isdigit(*state.current))
      {
        return token_result_type::error({
          position,
          U"Missing digits after `.'."
        });
      }
      do
      {
        const auto c = state.advance();

        if (c != '_')
        {
          buffer.append(1, c);
        }
      }
      while (!state.eof() && is_num_part(*state.current));
      if (state.peek_read('e') || state.peek_read('E'))
      {
        buffer.append(1, 'e');
        if (state.peek('+') || state.peek('-'))
        {
          buffer.append(1, state.advance());
        }
        if (state.eof() || !std::isdigit(*state.current))
        {
          return token_result_type::error({
            position,
            U"Missing digits after `e'."
          });
        }
        do
        {
          const auto c = state.advance();

          if (c != '_')
          {
            buffer.append(1, c);
          }
        }
        while (!state.eof() && is_num_part(*state.current));
      }
    }

    return make_token(position, kind, buffer);
  }

  static token_result_type
  lex_operator(State& state)
  {
    const auto position = state.position;
    const auto c = state.advance();
    cst::Kind kind;

    switch (c)
    {
      case '.':
        if (state.peek_read('.'))
        {
          if (!state.peek_read('.'))
          {
            return token_result_type::error({
              position,
              U"Unexpected `..'."
            });
          }
          kind = cst::Kind::Spread;
        } else {
          kind = cst::Kind::Dot;
        }
        break;

      case ',':
        kind = cst::Kind::Comma;
        break;

      case ':':
        kind = cst::Kind::Colon;
        break;

      case ';':
        kind = cst::Kind::Semicolon;
        break;

      case '!':
        kind = state.peek_read('=') ? cst::Kind::Ne : cst::Kind::Not;
        break;

      case '~':
        kind = cst::Kind::BitwiseNot;
        break;

      case '^':
        kind = cst::Kind::BitwiseXor;
        break;

      case '=':
        kind =
          state.peek_read('=') ?
          cst::Kind::Eq :
          state.peek_read('>') ?
          cst::Kind::FatArrow :
          cst::Kind::Assign;
        break;

      case '+':
        kind = cst::Kind::Add;
        break;

      case '-':
        kind = state.peek_read('>') ? cst::Kind::Arrow : cst::Kind::Sub;
        break;

      case '*':
        kind = cst::Kind::Mul;
        break;

      case '%':
        kind = cst::Kind::Mod;
        break;

      case '/':
        kind = cst::Kind::Div;
        break;

      case '&':
        kind = state.peek_read('&') ? cst::Kind::LogicalAnd : cst::Kind::And;
        break;

      case '|':
        kind = state.peek_read('|') ? cst::Kind::LogicalOr : cst::Kind::Or;
        break;

      case '<':
        kind = state.peek_read('<')
          ? cst::Kind::LeftShift
          : state.peek_read('=')
          ? cst::Kind::Lte
          : cst::Kind::Lt;
        break;

      case '>':
        kind = state.peek_read('>')
          ? cst::Kind::RightShift
          : state.peek_read('=')
          ? cst::Kind::Gte
          : cst::Kind::Gt;
        break;

      case '?':
        if (!state.peek_read('.'))
        {
          return token_result_type::error({
            position,
            U"Unexpected `?'.",
          });
        }
        kind = cst::Kind::ConditionalDot;
        break;

      default:
        return token_result_type::error({
          position,
          U"Unexpected input."
        });
    }

    return make_token(position, kind);
  }
}
