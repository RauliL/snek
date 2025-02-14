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

#include "snek/macros.hpp"
#include "snek/parser/node.hpp"

namespace snek::parser
{
  class Token final : public Node
  {
  public:
    DEFAULT_COPY_AND_ASSIGN(Token);

    enum class Kind
    {
      // End of input.
      Eof,
      NewLine,
      Indent,
      Dedent,

      // Separators.
      LeftParen,
      RightParen,
      LeftBracket,
      RightBracket,
      LeftBrace,
      RightBrace,
      Dot,
      ConditionalDot,
      Comma,
      Colon,
      Semicolon,
      Arrow,
      FatArrow,
      Spread,
      Ternary,

      // Literals.
      Id,
      Int,
      Float,
      String,

      // Operators.
      Add,
      Sub,
      Mul,
      Div,
      Mod,
      Not,
      BitwiseAnd,
      BitwiseOr,
      BitwiseNot,
      BitwiseXor,
      Equal,
      NotEqual,
      LessThan,
      GreaterThan,
      LessThanEqual,
      GreaterThanEqual,
      LeftShift,
      RightShift,
      LogicalAnd,
      LogicalOr,
      Increment,
      Decrement,
      NullCoalescing,

      // Assignment operators.
      Assign,
      AssignAdd,
      AssignSub,
      AssignMul,
      AssignDiv,
      AssignMod,
      AssignBitwiseAnd,
      AssignBitwiseOr,
      AssignBitwiseXor,
      AssignLeftShift,
      AssignRightShift,
      AssignLogicalAnd,
      AssignLogicalOr,
      AssignNullCoalescing,

      // Reserved keywords.
      KeywordAs,
      KeywordBreak,
      KeywordConst,
      KeywordContinue,
      KeywordFalse,
      KeywordElse,
      KeywordFor,
      KeywordFrom,
      KeywordExport,
      KeywordIf,
      KeywordImport,
      KeywordLet,
      KeywordNull,
      KeywordPass,
      KeywordReturn,
      KeywordTrue,
      KeywordType,
      KeywordWhile,
    };

    Token(
      const std::optional<Position>& position = std::nullopt,
      Kind kind = Kind::Eof,
      const std::optional<std::u32string>& text = std::nullopt
    )
      : Node(position)
      , m_kind(kind)
      , m_text(text) {}

    inline Kind kind() const
    {
      return m_kind;
    }

    inline const std::optional<std::u32string>& text() const
    {
      return m_text;
    }

    static std::u32string ToString(Kind kind);

    std::u32string ToString() const override;

  private:
    Kind m_kind;
    std::optional<std::u32string> m_text;
  };
}
