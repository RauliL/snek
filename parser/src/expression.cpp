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
#include <peelo/unicode/encoding/utf8.hpp>

#include "snek/parser/element.hpp"
#include "snek/parser/error.hpp"
#include "snek/parser/field.hpp"
#include "snek/parser/statement.hpp"
#include "snek/parser/type.hpp"
#include "snek/parser/utils.hpp"

namespace snek::parser
{
  using lex_function = std::function<expression_ptr(Lexer&)>;

  template<class T>
  static std::vector<T>
  ParseMultiple(
    const std::optional<Position>& position,
    Lexer& lexer,
    T (*callback)(Lexer&),
    TokenKind closing_token,
    const char32_t* description
  )
  {
    std::vector<T> result;

    for (;;)
    {
      if (lexer.PeekToken(TokenKind::Eof))
      {
        throw SyntaxError{
          position,
          std::u32string(U"Unterminated ") +
          description +
          U"; Missing " +
          ToString(closing_token) +
          U"."
        };
      }
      else if (lexer.PeekReadToken(closing_token))
      {
        break;
      }
      result.push_back(callback(lexer));
      if (
        !lexer.PeekToken(TokenKind::Comma) &&
        !lexer.PeekToken(closing_token)
      )
      {
        throw SyntaxError{
          position,
          std::u32string(U"Unterminated ") +
          description +
          U"; Missing " +
          ToString(closing_token) +
          U"."
        };
      }
      if (!lexer.PeekReadToken(TokenKind::Comma))
      {
        lexer.ReadToken(closing_token);
        break;
      }
    }

    return result;
  }

  // Special hack that tries to detect whether an function expression is coming
  // next.
  static inline bool
  PeekFunction(Lexer& lexer)
  {
    return (
      // (...arg)
      (
        lexer.PeekToken(TokenKind::Spread)
      ) ||
      // ()
      (
        lexer.PeekToken(TokenKind::RightBrace)
      ) ||
      // ():
      (
        lexer.PeekToken(TokenKind::RightParen) &&
        lexer.PeekNextButOneToken(TokenKind::Colon)
      ) ||
      // () ->
      (
        lexer.PeekToken(TokenKind::RightParen) &&
        lexer.PeekNextButOneToken(TokenKind::Arrow)
      ) ||
      // () =>
      (
        lexer.PeekToken(TokenKind::RightParen) &&
        lexer.PeekNextButOneToken(TokenKind::FatArrow)
      ) ||
      // (arg:
      (
        lexer.PeekToken(TokenKind::Id) &&
        lexer.PeekNextButOneToken(TokenKind::Colon)
      ) ||
      // (arg,
      (
        lexer.PeekToken(TokenKind::Id) &&
        lexer.PeekNextButOneToken(TokenKind::Comma)
      )
    );
  }

  static Parameter
  ToParameter(const expression_ptr& expression)
  {
    if (expression->Is(ExpressionKind::Id))
    {
      return Parameter(
        expression->position,
        As<Expression::Id>(expression)->identifier
      );
    }
    else if (expression->Is(ExpressionKind::Assign))
    {
      const auto assign = As<Expression::Assign>(expression);

      if (!assign->op && assign->variable->Is(ExpressionKind::Id))
      {
        return Parameter(
          expression->position,
          As<Expression::Id>(assign->variable)->identifier
        );
      }
    }

    throw SyntaxError{
      expression->position,
      U"Unexpected `" +
      expression->ToString() +
      U"'; Missing function."
    };
  }

  static inline expression_ptr
  ParseArgument(Lexer& lexer)
  {
    return lexer.PeekReadToken(TokenKind::Spread)
      ? std::make_shared<Expression::Spread>(
          lexer.position(),
          Expression::Parse(lexer)
        )
      : Expression::Parse(lexer);
  }

  static inline std::vector<expression_ptr>
  ParseArgumentList(const std::optional<Position>& position, Lexer& lexer)
  {
    return ParseMultiple(
      position,
      lexer,
      ParseArgument,
      TokenKind::RightParen,
      U"argument list"
    );
  }

  static expression_ptr
  ParseInt(const Token& token)
  {
    using peelo::unicode::encoding::utf8::encode;

    // TODO: Implement Unicode version of std::strtoll.
    // TODO: Implement different bases.
    const auto value = std::strtoll(
      encode(*token.text).c_str(),
      nullptr,
      10
    );

    return std::make_shared<Expression::Int>(token.position, value);
  }

  static expression_ptr
  ParseFloat(const Token& token)
  {
    using peelo::unicode::encoding::utf8::encode;

    // TODO: Implement Unicode version of std::strtod.
    const auto value = std::strtod(encode(*token.text).c_str(), nullptr);

    return std::make_shared<Expression::Float>(token.position, value);
  }

  static expression_ptr
  ParseFunctionRest(
    const std::optional<Position>& position,
    const std::vector<Parameter>& parameters,
    Lexer& lexer
  )
  {
    type_ptr return_type;

    if (lexer.PeekReadToken(TokenKind::Arrow))
    {
      return_type = Type::Parse(lexer);
    }

    return std::make_shared<Expression::Function>(
      position,
      Signature{ parameters, return_type },
      Statement::ParseFunctionBody(lexer)
    );
  }

  static inline expression_ptr
  ParseFunction(const std::optional<Position>& position, Lexer& lexer)
  {
    return ParseFunctionRest(
      position,
      Parameter::ParseList(lexer, false),
      lexer
    );
  }

  expression_ptr
  Expression::ParseFunction(Lexer& lexer)
  {
    return ParseFunctionRest(
      lexer.position(),
      Parameter::ParseList(lexer, true),
      lexer
    );
  }

  static expression_ptr
  ParseParenthesized(const std::optional<Position>& position, Lexer& lexer)
  {
    if (!PeekFunction(lexer))
    {
      const auto expression = Expression::Parse(lexer);

      lexer.ReadToken(TokenKind::RightParen);
      if (
        lexer.PeekToken(TokenKind::Arrow) ||
        lexer.PeekToken(TokenKind::FatArrow) ||
        lexer.PeekToken(TokenKind::Colon)
      )
      {
        return ParseFunctionRest(position, { ToParameter(expression) }, lexer);
      }

      return expression;
    }

    return ParseFunction(position, lexer);
  }

  static expression_ptr
  ParsePrimary(Lexer& lexer)
  {
    const auto token = lexer.ReadToken();

    switch (token.kind)
    {
      case TokenKind::Eof:
        throw SyntaxError{
          token.position,
          U"Unexpected end of input; Missing expression."
        };

      case TokenKind::KeywordTrue:
        return std::make_shared<Expression::Boolean>(token.position, true);

      case TokenKind::KeywordFalse:
        return std::make_shared<Expression::Boolean>(token.position, false);

      case TokenKind::KeywordNull:
        return std::make_shared<Expression::Null>(token.position);

      case TokenKind::Int:
        return ParseInt(token);

      case TokenKind::Float:
        return ParseFloat(token);

      case TokenKind::String:
        return std::make_shared<Expression::String>(
          token.position,
          *token.text
        );

      case TokenKind::LeftBracket:
        return std::make_shared<Expression::List>(
          token.position,
          ParseMultiple(
            token.position,
            lexer,
            Element::Parse,
            TokenKind::RightBracket,
            U"list"
          )
        );

      case TokenKind::LeftBrace:
        return std::make_shared<Expression::Record>(
          token.position,
          ParseMultiple(
            token.position,
            lexer,
            Field::Parse,
            TokenKind::RightBrace,
            U"record"
          )
        );

      case TokenKind::Id:
        return std::make_shared<Expression::Id>(token.position, *token.text);

      case TokenKind::LeftParen:
        return ParseParenthesized(token.position, lexer);

      default:
        throw SyntaxError{
          token.position,
          U"Unexpected " + token.ToString() + U"; Missing expression."
        };
    }
  }

  static expression_ptr
  ParseUnary(Lexer& lexer)
  {
    if (
      lexer.PeekToken(TokenKind::Not) ||
      lexer.PeekToken(TokenKind::Add) ||
      lexer.PeekToken(TokenKind::Sub) ||
      lexer.PeekToken(TokenKind::BitwiseNot)
    )
    {
      const auto token = lexer.ReadToken();

      return std::make_shared<Expression::Unary>(
        token.position,
        static_cast<UnaryOp>(token.kind),
        ParseUnary(lexer)
      );
    }
    else if (lexer.PeekToken(TokenKind::Increment))
    {
      const auto token = lexer.ReadToken();

      return std::make_shared<Expression::Increment>(
        token.position,
        ParseUnary(lexer),
        true
      );
    }
    else if (lexer.PeekToken(TokenKind::Decrement))
    {
      const auto token = lexer.ReadToken();

      return std::make_shared<Expression::Decrement>(
        token.position,
        ParseUnary(lexer),
        true
      );
    }

    auto expression = ParsePrimary(lexer);

    while (
      lexer.PeekToken(TokenKind::Dot) ||
      lexer.PeekToken(TokenKind::LeftParen) ||
      lexer.PeekToken(TokenKind::LeftBracket) ||
      lexer.PeekToken(TokenKind::ConditionalDot) ||
      lexer.PeekToken(TokenKind::Increment) ||
      lexer.PeekToken(TokenKind::Decrement)
    )
    {
      const auto token = lexer.ReadToken();

      switch (token.kind)
      {
        case TokenKind::Dot:
          expression = std::make_shared<Expression::Property>(
            token.position,
            expression,
            lexer.ReadId(),
            false
          );
          break;

        case TokenKind::LeftParen:
          expression = std::make_shared<Expression::Call>(
            token.position,
            expression,
            ParseArgumentList(token.position, lexer),
            false
          );
          break;

        case TokenKind::LeftBracket:
          expression = std::make_shared<Expression::Subscript>(
            token.position,
            expression,
            Expression::Parse(lexer),
            false
          );
          lexer.ReadToken(TokenKind::RightBracket);
          break;

        case TokenKind::ConditionalDot:
          if (lexer.PeekReadToken(TokenKind::LeftParen))
          {
            expression = std::make_shared<Expression::Call>(
              token.position,
              expression,
              ParseArgumentList(token.position, lexer),
              true
            );
          }
          else if (lexer.PeekReadToken(TokenKind::LeftBracket))
          {
            expression = std::make_shared<Expression::Subscript>(
              token.position,
              expression,
              Expression::Parse(lexer),
              true
            );
            lexer.ReadToken(TokenKind::RightBracket);
          } else {
            expression = std::make_shared<Expression::Property>(
              token.position,
              expression,
              lexer.ReadId(),
              true
            );
          }
          break;

        case TokenKind::Increment:
          expression = std::make_shared<Expression::Increment>(
            token.position,
            expression,
            false
          );
          break;

        case TokenKind::Decrement:
          expression = std::make_shared<Expression::Decrement>(
            token.position,
            expression,
            false
          );
          break;

        default:
          break;
      }
    }

    return expression;
  }

  static expression_ptr
  ParseBinary(
    Lexer& lexer,
    const lex_function& next,
    std::initializer_list<TokenKind> expected_tokens
  )
  {
    auto expression = next(lexer);

    while (lexer.PeekToken(expected_tokens))
    {
      const auto op = static_cast<BinaryOp>(lexer.ReadToken().kind);
      const auto operand = next(lexer);

      expression = std::make_shared<Expression::Binary>(
        expression,
        op,
        operand
      );
    }

    return expression;
  }

  static inline expression_ptr
  ParseMultiplicative(Lexer& lexer)
  {
    return ParseBinary(
      lexer,
      ParseUnary,
      {
        TokenKind::Mul,
        TokenKind::Div,
        TokenKind::Mod,
      }
    );
  }

  static inline expression_ptr
  ParseAdditive(Lexer& lexer)
  {
    return ParseBinary(
      lexer,
      ParseMultiplicative,
      {
        TokenKind::Add,
        TokenKind::Sub,
      }
    );
  }

  static inline expression_ptr
  ParseShift(Lexer& lexer)
  {
    return ParseBinary(
      lexer,
      ParseAdditive,
      {
        TokenKind::LeftShift,
        TokenKind::RightShift,
      }
    );
  }

  static inline expression_ptr
  ParseRelational(Lexer& lexer)
  {
    return ParseBinary(
      lexer,
      ParseShift,
      {
        TokenKind::LessThan,
        TokenKind::GreaterThan,
        TokenKind::LessThanEqual,
        TokenKind::GreaterThanEqual,
      }
    );
  }

  static inline expression_ptr
  ParseEquality(Lexer& lexer)
  {
    return ParseBinary(
      lexer,
      ParseRelational,
      {
        TokenKind::Equal,
        TokenKind::NotEqual,
      }
    );
  }

  static inline expression_ptr
  ParseBitwiseAnd(Lexer& lexer)
  {
    return ParseBinary(
      lexer,
      ParseEquality,
      { TokenKind::BitwiseAnd }
    );
  }

  static inline expression_ptr
  ParseBitwiseXor(Lexer& lexer)
  {
    return ParseBinary(
      lexer,
      ParseBitwiseAnd,
      { TokenKind::BitwiseXor }
    );
  }

  static inline expression_ptr
  ParseBitwiseOr(Lexer& lexer)
  {
    return ParseBinary(
      lexer,
      ParseBitwiseXor,
      { TokenKind::BitwiseOr }
    );
  }

  static inline expression_ptr
  ParseLogicalAnd(Lexer& lexer)
  {
    return ParseBinary(
      lexer,
      ParseBitwiseOr,
      { TokenKind::LogicalAnd }
    );
  }

  static inline expression_ptr
  ParseLogicalOr(Lexer& lexer)
  {
    return ParseBinary(
      lexer,
      ParseLogicalAnd,
      {
        TokenKind::LogicalOr,
        TokenKind::NullCoalescing,
      }
    );
  }

  expression_ptr
  Expression::ParseTernary(Lexer& lexer)
  {
    const auto expression = ParseLogicalOr(lexer);

    if (lexer.PeekReadToken(TokenKind::Ternary))
    {
      const auto then_expression = Parse(lexer);

      lexer.ReadToken(TokenKind::Colon);

      return std::make_shared<Ternary>(
        expression->position,
        expression,
        then_expression,
        Parse(lexer)
      );
    }

    return expression;
  }

  expression_ptr
  Expression::Parse(Lexer& lexer)
  {
    const auto expression = ParseTernary(lexer);

    if (
      lexer.PeekToken(TokenKind::Assign) ||
      lexer.PeekToken(TokenKind::AssignAdd) ||
      lexer.PeekToken(TokenKind::AssignSub) ||
      lexer.PeekToken(TokenKind::AssignMul) ||
      lexer.PeekToken(TokenKind::AssignDiv) ||
      lexer.PeekToken(TokenKind::AssignMod) ||
      lexer.PeekToken(TokenKind::AssignBitwiseAnd) ||
      lexer.PeekToken(TokenKind::AssignBitwiseOr) ||
      lexer.PeekToken(TokenKind::AssignBitwiseXor) ||
      lexer.PeekToken(TokenKind::AssignLeftShift) ||
      lexer.PeekToken(TokenKind::AssignRightShift) ||
      lexer.PeekToken(TokenKind::AssignLogicalAnd) ||
      lexer.PeekToken(TokenKind::AssignLogicalOr) ||
      lexer.PeekToken(TokenKind::AssignNullCoalescing)
    )
    {
      const auto token = lexer.ReadToken();
      const auto kind = token.kind;
      const auto value = Parse(lexer);

      if (!expression->IsAssignable())
      {
        throw SyntaxError{
          expression->position,
          U"Cannot assign to `" + expression->ToString() + U"'."
        };
      }

      return std::make_shared<Expression::Assign>(
        token.position,
        expression,
        value,
        kind == TokenKind::Assign
          ? std::nullopt
          : std::make_optional(static_cast<AssignOp>(kind))
      );
    }

    return expression;
  }

  std::u32string
  Expression::Assign::ToString() const
  {
    auto result = variable->ToString();

    if (op)
    {
      result.append(1, U' ').append(parser::ToString(*op));
    }

    return result.append(U"= ").append(value->ToString());
  }

  std::u32string
  Expression::Binary::ToString() const
  {
    return left->ToString()
      .append(1, U' ')
      .append(parser::ToString(op))
      .append(1, U' ')
      .append(right->ToString());
  }

  std::u32string
  Expression::Call::ToString() const
  {
    auto result = expression->ToString();

    if (conditional)
    {
      result.append(U"?.");
    }

    return result.append(
      utils::Join<expression_ptr, std::vector<expression_ptr>::const_iterator>(
        std::begin(arguments),
        std::end(arguments),
        [](const auto& argument)
        {
          return argument->ToString();
        },
        U'(',
        U')'
      )
    );
  }

  std::u32string
  Expression::Decrement::ToString() const
  {
    return pre
      ? U"--" + variable->ToString()
      : variable->ToString() + U"--";
  }

  std::u32string
  Expression::Int::ToString() const
  {
    return utils::IntToString(value);
  }

  std::u32string
  Expression::Float::ToString() const
  {
    return utils::DoubleToString(value);
  }

  std::u32string
  Expression::Increment::ToString() const
  {
    return pre
      ? U"++" + variable->ToString()
      : variable->ToString() + U"++";
  }

  std::u32string
  Expression::List::ToString() const
  {
    return utils::Join<
      element_ptr,
      std::vector<element_ptr>::const_iterator
    >(
      std::begin(elements),
      std::end(elements),
      [](const auto& element)
      {
        return element->ToString();
      },
      U'[',
      U']'
    );
  }

  std::u32string
  Expression::Property::ToString() const
  {
    auto result = expression->ToString();

    if (conditional)
    {
      result.append(1, U'?');
    }

    return result.append(1, U'.').append(name);
  }

  bool
  Expression::Record::IsAssignable() const
  {
    for (const auto& field : fields)
    {
      const auto kind = field->kind;

      if (kind == FieldKind::Spread)
      {
        if (!static_cast<const Field::Spread*>(
          field.get()
        )->expression->IsAssignable())
        {
          return false;
        }
      }
      else if (kind != FieldKind::Named && kind != FieldKind::Shorthand)
      {
        return false;
      }
    }

    return true;
  }

  std::u32string
  Expression::Record::ToString() const
  {
    return utils::Join<value_type, container_type::const_iterator>(
      std::begin(fields),
      std::end(fields),
      [](const auto& field)
      {
        return field->ToString();
      },
      U'{',
      U'}'
    );
  }

  std::u32string
  Expression::String::ToString() const
  {
    return utils::ToJsonString(value);
  }

  std::u32string
  Expression::Subscript::ToString() const
  {
    auto result = expression->ToString();

    if (conditional)
    {
      result.append(U"?.");
    }

    return result.append(1, U'[').append(index->ToString()).append(1, U']');
  }

  std::u32string
  Expression::Ternary::ToString() const
  {
    return condition->ToString()
      .append(U" ? ")
      .append(then_expression->ToString())
      .append(U" : ")
      .append(else_expression->ToString());
  }

  std::u32string
  Expression::Unary::ToString() const
  {
    return parser::ToString(op).append(operand->ToString());
  }
}
