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
#include "snek/parser/parameter.hpp"
#include "snek/parser/statement.hpp"
#include "snek/parser/type.hpp"
#include "snek/parser/utils.hpp"

namespace snek::parser::expression
{
  template<class T>
  static inline const T*
  As(const ptr& expression)
  {
    return static_cast<const T*>(expression.get());
  }

  template<class T>
  static std::vector<T>
  ParseMultiple(
    const std::optional<Position>& position,
    Lexer& lexer,
    T (*callback)(Lexer&),
    Token::Kind closing_token,
    const char32_t* description
  )
  {
    std::vector<T> result;

    for (;;)
    {
      if (lexer.PeekToken(Token::Kind::Eof))
      {
        throw Error{
          position,
          std::u32string(U"Unterminated ") +
          description +
          U"; Missing " +
          Token::ToString(closing_token) +
          U"."
        };
      }
      else if (lexer.PeekReadToken(closing_token))
      {
        break;
      }
      result.push_back(callback(lexer));
      if (
        !lexer.PeekToken(Token::Kind::Comma) &&
        !lexer.PeekToken(closing_token)
      )
      {
        throw Error{
          position,
          std::u32string(U"Unterminated ") +
          description +
          U"; Missing " +
          Token::ToString(closing_token) +
          U"."
        };
      }
      if (!lexer.PeekReadToken(Token::Kind::Comma))
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
        lexer.PeekToken(Token::Kind::Spread)
      ) ||
      // ()
      (
        lexer.PeekToken(Token::Kind::RightBrace)
      ) ||
      // ():
      (
        lexer.PeekToken(Token::Kind::RightParen) &&
        lexer.PeekNextButOneToken(Token::Kind::Colon)
      ) ||
      // () ->
      (
        lexer.PeekToken(Token::Kind::RightParen) &&
        lexer.PeekNextButOneToken(Token::Kind::Arrow)
      ) ||
      // () =>
      (
        lexer.PeekToken(Token::Kind::RightParen) &&
        lexer.PeekNextButOneToken(Token::Kind::FatArrow)
      ) ||
      // (arg:
      (
        lexer.PeekToken(Token::Kind::Id) &&
        lexer.PeekNextButOneToken(Token::Kind::Colon)
      ) ||
      // (arg,
      (
        lexer.PeekToken(Token::Kind::Id) &&
        lexer.PeekNextButOneToken(Token::Kind::Comma)
      )
    );
  }

  static parameter::ptr
  ToParameter(const ptr& expression)
  {
    if (expression->kind() == Kind::Id)
    {
      return std::make_shared<parameter::Base>(
        expression->position(),
        As<Id>(expression)->identifier()
      );
    }
    else if (expression->kind() == Kind::Assign)
    {
      const auto assign = As<Assign>(expression);
      const auto variable = assign->variable();

      if (!assign->op() && variable->kind() == Kind::Id)
      {
        return std::make_shared<parameter::Base>(
          expression->position(),
          As<Id>(variable)->identifier()
        );
      }
    }

    throw Error{
      expression->position(),
      U"Unexpected `" +
      expression->ToString() +
      U"'; Missing function."
    };
  }

  static inline ptr
  ParseArgument(Lexer& lexer)
  {
    return lexer.PeekReadToken(Token::Kind::Spread)
      ? std::make_shared<Spread>(lexer.position(), Parse(lexer))
      : Parse(lexer);
  }

  static inline std::vector<ptr>
  ParseArgumentList(const std::optional<Position>& position, Lexer& lexer)
  {
    return ParseMultiple(
      position,
      lexer,
      ParseArgument,
      Token::Kind::RightParen,
      U"argument list"
    );
  }

  static ptr
  ParseInt(const Token& token)
  {
    using peelo::unicode::encoding::utf8::encode;

    // TODO: Implement Unicode version of std::strtoll.
    // TODO: Implement different bases.
    const auto value = std::strtoll(
      encode(*token.text()).c_str(),
      nullptr,
      10
    );

    return std::make_shared<Int>(token.position(), value);
  }

  static ptr
  ParseFloat(const Token& token)
  {
    using peelo::unicode::encoding::utf8::encode;

    // TODO: Implement Unicode version of std::strtod.
    const auto value = std::strtod(encode(*token.text()).c_str(), nullptr);

    return std::make_shared<Float>(token.position(), value);
  }

  static ptr
  ParseFunctionRest(
    const std::optional<Position>& position,
    const std::vector<parameter::ptr>& parameters,
    Lexer& lexer
  )
  {
    type::ptr return_type;

    if (lexer.PeekReadToken(Token::Kind::Arrow))
    {
      return_type = type::Parse(lexer);
    }

    return std::make_shared<Function>(
      position,
      parameters,
      return_type,
      statement::ParseFunctionBody(lexer)
    );
  }

  static inline ptr
  ParseFunction(const std::optional<Position>& position, Lexer& lexer)
  {
    return ParseFunctionRest(
      position,
      parameter::ParseList(lexer, false),
      lexer)
    ;
  }

  static ptr
  ParseParenthesized(const std::optional<Position>& position, Lexer& lexer)
  {
    if (!PeekFunction(lexer))
    {
      const auto expression = Parse(lexer);

      lexer.ReadToken(Token::Kind::RightParen);
      if (
        lexer.PeekToken(Token::Kind::Arrow) ||
        lexer.PeekToken(Token::Kind::FatArrow) ||
        lexer.PeekToken(Token::Kind::Colon)
      )
      {
        return ParseFunctionRest(position, { ToParameter(expression) }, lexer);
      }

      return expression;
    }

    return ParseFunction(position, lexer);
  }

  static ptr
  ParsePrimary(Lexer& lexer)
  {
    const auto token = lexer.ReadToken();

    switch (token.kind())
    {
      case Token::Kind::Eof:
        throw Error{
          token.position(),
          U"Unexpected end of input; Missing expression."
        };

      case Token::Kind::KeywordTrue:
        return std::make_shared<Boolean>(token.position(), true);

      case Token::Kind::KeywordFalse:
        return std::make_shared<Boolean>(token.position(), false);

      case Token::Kind::KeywordNull:
        return std::make_shared<Null>(token.position());

      case Token::Kind::Int:
        return ParseInt(token);

      case Token::Kind::Float:
        return ParseFloat(token);

      case Token::Kind::String:
        return std::make_shared<String>(token.position(), *token.text());

      case Token::Kind::LeftBracket:
        return std::make_shared<List>(
          token.position(),
          ParseMultiple(
            token.position(),
            lexer,
            element::Parse,
            Token::Kind::RightBracket,
            U"list"
          )
        );

      case Token::Kind::LeftBrace:
        return std::make_shared<Record>(
          token.position(),
          ParseMultiple(
            token.position(),
            lexer,
            field::Parse,
            Token::Kind::RightBrace,
            U"record"
          )
        );

      case Token::Kind::Id:
        return std::make_shared<Id>(token.position(), *token.text());

      case Token::Kind::LeftParen:
        return ParseParenthesized(token.position(), lexer);

      default:
        throw Error{
          token.position(),
          U"Unexpected " + token.ToString() + U"; Missing expression."
        };
    }
  }

  static ptr
  ParseUnary(Lexer& lexer)
  {
    if (
      lexer.PeekToken(Token::Kind::Not) ||
      lexer.PeekToken(Token::Kind::Add) ||
      lexer.PeekToken(Token::Kind::Sub) ||
      lexer.PeekToken(Token::Kind::BitwiseNot)
    )
    {
      const auto token = lexer.ReadToken();

      return std::make_shared<Unary>(
        token.position(),
        static_cast<Unary::Operator>(token.kind()),
        ParseUnary(lexer)
      );
    }
    else if (lexer.PeekToken(Token::Kind::Increment))
    {
      const auto token = lexer.ReadToken();

      return std::make_shared<Increment>(
        token.position(),
        ParseUnary(lexer),
        true
      );
    }
    else if (lexer.PeekToken(Token::Kind::Decrement))
    {
      const auto token = lexer.ReadToken();

      return std::make_shared<Decrement>(
        token.position(),
        ParseUnary(lexer),
        true
      );
    }

    auto expression = ParsePrimary(lexer);

    while (
      lexer.PeekToken(Token::Kind::Dot) ||
      lexer.PeekToken(Token::Kind::LeftParen) ||
      lexer.PeekToken(Token::Kind::LeftBracket) ||
      lexer.PeekToken(Token::Kind::ConditionalDot) ||
      lexer.PeekToken(Token::Kind::Increment) ||
      lexer.PeekToken(Token::Kind::Decrement)
    )
    {
      const auto token = lexer.ReadToken();

      switch (token.kind())
      {
        case Token::Kind::Dot:
          expression = std::make_shared<Property>(
            token.position(),
            expression,
            lexer.ReadId(),
            false
          );
          break;

        case Token::Kind::LeftParen:
          expression = std::make_shared<Call>(
            token.position(),
            expression,
            ParseArgumentList(token.position(), lexer),
            false
          );
          break;

        case Token::Kind::LeftBracket:
          expression = std::make_shared<Subscript>(
            token.position(),
            expression,
            Parse(lexer),
            false
          );
          lexer.ReadToken(Token::Kind::RightBracket);
          break;

        case Token::Kind::ConditionalDot:
          if (lexer.PeekReadToken(Token::Kind::LeftParen))
          {
            expression = std::make_shared<Call>(
              token.position(),
              expression,
              ParseArgumentList(token.position(), lexer),
              true
            );
          }
          else if (lexer.PeekReadToken(Token::Kind::LeftBracket))
          {
            expression = std::make_shared<Subscript>(
              token.position(),
              expression,
              Parse(lexer),
              true
            );
            lexer.ReadToken(Token::Kind::RightBracket);
          } else {
            expression = std::make_shared<Property>(
              token.position(),
              expression,
              lexer.ReadId(),
              true
            );
          }
          break;

        case Token::Kind::Increment:
          expression = std::make_shared<Increment>(
            token.position(),
            expression,
            false
          );
          break;

        case Token::Kind::Decrement:
          expression = std::make_shared<Decrement>(
            token.position(),
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

  static ptr
  ParseMultiplicative(Lexer& lexer)
  {
    auto expression = ParseUnary(lexer);

    while (
      lexer.PeekToken(Token::Kind::Mul) ||
      lexer.PeekToken(Token::Kind::Div) ||
      lexer.PeekToken(Token::Kind::Mod)
    )
    {
      const auto op = static_cast<Binary::Operator>(lexer.ReadToken().kind());
      const auto operand = ParseUnary(lexer);

      expression = std::make_shared<Binary>(expression, op, operand);
    }

    return expression;
  }

  static ptr
  ParseAdditive(Lexer& lexer)
  {
    auto expression = ParseMultiplicative(lexer);

    while (
      lexer.PeekToken(Token::Kind::Add) ||
      lexer.PeekToken(Token::Kind::Sub)
    )
    {
      const auto op = static_cast<Binary::Operator>(lexer.ReadToken().kind());
      const auto operand = ParseMultiplicative(lexer);

      expression = std::make_shared<Binary>(expression, op, operand);
    }

    return expression;
  }

  static ptr
  ParseShift(Lexer& lexer)
  {
    auto expression = ParseAdditive(lexer);

    while (
      lexer.PeekToken(Token::Kind::LeftShift) ||
      lexer.PeekToken(Token::Kind::RightShift)
    )
    {
      const auto op = static_cast<Binary::Operator>(lexer.ReadToken().kind());
      const auto operand = ParseAdditive(lexer);

      expression = std::make_shared<Binary>(expression, op, operand);
    }

    return expression;
  }

  static ptr
  ParseRelational(Lexer& lexer)
  {
    auto expression = ParseShift(lexer);

    while (
      lexer.PeekToken(Token::Kind::LessThan) ||
      lexer.PeekToken(Token::Kind::GreaterThan) ||
      lexer.PeekToken(Token::Kind::LessThanEqual) ||
      lexer.PeekToken(Token::Kind::GreaterThanEqual)
    )
    {
      const auto op = static_cast<Binary::Operator>(lexer.ReadToken().kind());
      const auto operand = ParseShift(lexer);

      expression = std::make_shared<Binary>(expression, op, operand);
    }

    return expression;
  }

  static ptr
  ParseEquality(Lexer& lexer)
  {
    auto expression = ParseRelational(lexer);

    while (
      lexer.PeekToken(Token::Kind::Equal) ||
      lexer.PeekToken(Token::Kind::NotEqual)
    )
    {
      const auto op = static_cast<Binary::Operator>(lexer.ReadToken().kind());
      const auto operand = ParseRelational(lexer);

      expression = std::make_shared<Binary>(expression, op, operand);
    }

    return expression;
  }

  static ptr
  ParseBitwiseAnd(Lexer& lexer)
  {
    auto expression = ParseEquality(lexer);

    while (lexer.PeekToken(Token::Kind::BitwiseAnd))
    {
      const auto op = static_cast<Binary::Operator>(lexer.ReadToken().kind());
      const auto operand = ParseEquality(lexer);

      expression = std::make_shared<Binary>(expression, op, operand);
    }

    return expression;
  }

  static ptr
  ParseBitwiseXor(Lexer& lexer)
  {
    auto expression = ParseBitwiseAnd(lexer);

    while (lexer.PeekToken(Token::Kind::BitwiseXor))
    {
      const auto op = static_cast<Binary::Operator>(lexer.ReadToken().kind());
      const auto operand = ParseBitwiseAnd(lexer);

      expression = std::make_shared<Binary>(expression, op, operand);
    }

    return expression;
  }

  static ptr
  ParseBitwiseOr(Lexer& lexer)
  {
    auto expression = ParseBitwiseXor(lexer);

    while (lexer.PeekToken(Token::Kind::BitwiseOr))
    {
      const auto op = static_cast<Binary::Operator>(lexer.ReadToken().kind());
      const auto operand = ParseBitwiseXor(lexer);

      expression = std::make_shared<Binary>(expression, op, operand);
    }

    return expression;
  }

  static ptr
  ParseLogicalAnd(Lexer& lexer)
  {
    auto expression = ParseBitwiseOr(lexer);

    while (lexer.PeekToken(Token::Kind::LogicalAnd))
    {
      const auto op = static_cast<Binary::Operator>(lexer.ReadToken().kind());
      const auto operand = ParseBitwiseOr(lexer);

      expression = std::make_shared<Binary>(expression, op, operand);
    }

    return expression;
  }

  static ptr
  ParseLogicalOr(Lexer& lexer)
  {
    auto expression = ParseLogicalAnd(lexer);

    while (lexer.PeekToken(Token::Kind::LogicalOr))
    {
      const auto op = static_cast<Binary::Operator>(lexer.ReadToken().kind());
      const auto operand = ParseLogicalAnd(lexer);

      expression = std::make_shared<Binary>(expression, op, operand);
    }

    return expression;
  }

  ptr
  ParseTernary(Lexer& lexer)
  {
    const auto expression = ParseLogicalOr(lexer);

    if (lexer.PeekReadToken(Token::Kind::Ternary))
    {
      const auto then_expression = Parse(lexer);

      lexer.ReadToken(Token::Kind::Colon);

      return std::make_shared<Ternary>(
        expression->position(),
        expression,
        then_expression,
        Parse(lexer)
      );
    }

    return expression;
  }

  ptr
  Parse(Lexer& lexer)
  {
    const auto expression = ParseTernary(lexer);

    if (
      lexer.PeekToken(Token::Kind::Assign) ||
      lexer.PeekToken(Token::Kind::AssignAdd) ||
      lexer.PeekToken(Token::Kind::AssignSub) ||
      lexer.PeekToken(Token::Kind::AssignMul) ||
      lexer.PeekToken(Token::Kind::AssignDiv) ||
      lexer.PeekToken(Token::Kind::AssignMod) ||
      lexer.PeekToken(Token::Kind::AssignBitwiseAnd) ||
      lexer.PeekToken(Token::Kind::AssignBitwiseOr) ||
      lexer.PeekToken(Token::Kind::AssignBitwiseXor) ||
      lexer.PeekToken(Token::Kind::AssignLeftShift) ||
      lexer.PeekToken(Token::Kind::AssignRightShift)
    )
    {
      const auto token = lexer.ReadToken();
      const auto kind = token.kind();
      const auto value = Parse(lexer);

      if (!expression->IsAssignable())
      {
        throw Error{
          expression->position(),
          U"Cannot assign to `" + expression->ToString() + U"'."
        };
      }

      return std::make_shared<Assign>(
        token.position(),
        expression,
        value,
        kind == Token::Kind::Assign
          ? std::nullopt
          : std::make_optional(static_cast<Assign::Operator>(kind))
      );
    }

    return expression;
  }

  std::u32string
  Assign::ToString(Operator op)
  {
    switch (op)
    {
      case Operator::Add:
        return U"+";

      case Operator::Sub:
        return U"-";

      case Operator::Mul:
        return U"*";

      case Operator::Div:
        return U"/";

      case Operator::Mod:
        return U"%";

      case Operator::BitwiseAnd:
        return U"&";

      case Operator::BitwiseOr:
        return U"|";

      case Operator::BitwiseXor:
        return U"^";

      case Operator::LeftShift:
        return U"<<";

      case Operator::RightShift:
        return U">>";
    }

    return U"unknown";
  }

  std::u32string
  Assign::ToString() const
  {
    std::u32string result(m_variable->ToString());

    if (m_op)
    {
      result.append(1, U' ').append(ToString(*m_op));
    }

    return result.append(U"= ").append(m_value->ToString());
  }

  std::u32string
  Binary::ToString(Operator op)
  {
    switch (op)
    {
      case Operator::Add:
        return U"+";

      case Operator::Sub:
        return U"-";

      case Operator::Mul:
        return U"*";

      case Operator::Div:
        return U"/";

      case Operator::Mod:
        return U"%";

      case Operator::BitwiseAnd:
        return U"&";

      case Operator::BitwiseOr:
        return U"|";

      case Operator::BitwiseXor:
        return U"^";

      case Operator::Equal:
        return U"==";

      case Operator::NotEqual:
        return U"!=";

      case Operator::LessThan:
        return U"<";

      case Operator::GreaterThan:
        return U">";

      case Operator::LessThanEqual:
        return U"<=";

      case Operator::GreaterThanEqual:
        return U">=";

      case Operator::LeftShift:
        return U"<<";

      case Operator::RightShift:
        return U">>";

      case Operator::LogicalAnd:
        return U"&&";

      case Operator::LogicalOr:
        return U"||";
    }

    return U"unknown";
  }

  std::u32string
  Binary::ToString() const
  {
    return m_left->ToString()
      .append(1, U' ')
      .append(ToString(m_op))
      .append(1, U' ')
      .append(m_right->ToString());
  }

  std::u32string
  Call::ToString() const
  {
    std::u32string result(m_expression->ToString());
    const auto size = m_arguments.size();

    if (m_conditional)
    {
      result.append(U"?.");
    }
    result.append(1, U'(');
    for (std::size_t i = 0; i < size; ++i)
    {
      if (i > 0)
      {
        result.append(U", ");
      }
      result.append(m_arguments[i]->ToString());
    }

    return result.append(1, U')');
  }

  std::u32string
  Decrement::ToString() const
  {
    return m_pre
      ? U"--" + m_variable->ToString()
      : m_variable->ToString() + U"--";
  }

  std::u32string
  Function::ToString() const
  {
    std::u32string result(1, U'(');
    bool first = true;

    for (const auto& parameter : m_parameters)
    {
      if (first)
      {
        first = false;
      } else {
        result.append(U", ");
      }
      result.append(parameter->ToString());
    }
    result.append(1, U')');
    if (m_return_type)
    {
      result.append(U": ");
      result.append(m_return_type->ToString());
    }

    return result;
  }

  std::u32string
  Int::ToString() const
  {
    return utils::IntToString(m_value);
  }

  std::u32string
  Float::ToString() const
  {
    return utils::DoubleToString(m_value);
  }

  std::u32string
  Increment::ToString() const
  {
    return m_pre
      ? U"++" + m_variable->ToString()
      : m_variable->ToString() + U"++";
  }

  std::u32string
  List::ToString() const
  {
    std::u32string result;
    bool first = true;

    result.append(1, U'[');
    for (const auto& element : m_elements)
    {
      if (first)
      {
        first = false;
      } else {
        result.append(U", ");
      }
      result.append(element->ToString());
    }
    result.append(1, U']');

    return result;
  }

  std::u32string
  Property::ToString() const
  {
    std::u32string result(m_expression->ToString());

    if (m_conditional)
    {
      result.append(1, U'?');
    }

    return result.append(1, U'.').append(m_name);
  }

  bool
  Record::IsAssignable() const
  {
    for (const auto& field : m_fields)
    {
      const auto kind = field->kind();

      if (kind == field::Kind::Spread)
      {
        if (!static_cast<const field::Spread*>(
          field.get()
        )->expression()->IsAssignable())
        {
          return false;
        }
      }
      else if (kind != field::Kind::Named && kind != field::Kind::Shorthand)
      {
        return false;
      }
    }

    return true;
  }

  std::u32string
  Record::ToString() const
  {
    std::u32string result;
    bool first = true;

    result.append(1, U'{');
    for (const auto& field : m_fields)
    {
      if (first)
      {
        first = false;
      } else {
        result.append(U", ");
      }
      result.append(field->ToString());
    }
    result.append(1, U'}');

    return result;
  }

  std::u32string
  String::ToString() const
  {
    return utils::ToJsonString(m_value);
  }

  std::u32string
  Subscript::ToString() const
  {
    std::u32string result(m_expression->ToString());

    if (m_conditional)
    {
      result.append(U"?.");
    }

    return result.append(1, U'[').append(m_index->ToString()).append(1, U']');
  }

  std::u32string
  Ternary::ToString() const
  {
    return m_condition->ToString()
      .append(U" ? ")
      .append(m_then_expression->ToString())
      .append(U" : ")
      .append(m_else_expression->ToString());
  }

  std::u32string
  Unary::ToString(Operator op)
  {
    switch (op)
    {
      case Operator::Add:
        return U"+";

      case Operator::BitwiseNot:
        return U"~";

      case Operator::Not:
        return U"!";

      case Operator::Sub:
        return U"-";
    }

    return U"unknown";
  }

  std::u32string
  Unary::ToString() const
  {
    return ToString(m_op).append(m_operand->ToString());
  }
}
