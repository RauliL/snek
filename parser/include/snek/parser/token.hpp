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
  enum class TokenKind
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

  enum class AssignOp
  {
    Add = static_cast<int>(TokenKind::AssignAdd),
    Sub = static_cast<int>(TokenKind::AssignSub),
    Mul = static_cast<int>(TokenKind::AssignMul),
    Div = static_cast<int>(TokenKind::AssignDiv),
    Mod = static_cast<int>(TokenKind::AssignMod),
    BitwiseAnd = static_cast<int>(TokenKind::AssignBitwiseAnd),
    BitwiseOr = static_cast<int>(TokenKind::AssignBitwiseOr),
    BitwiseXor = static_cast<int>(TokenKind::AssignBitwiseXor),
    LeftShift = static_cast<int>(TokenKind::AssignLeftShift),
    RightShift = static_cast<int>(TokenKind::AssignRightShift),
    LogicalAnd = static_cast<int>(TokenKind::AssignLogicalAnd),
    LogicalOr = static_cast<int>(TokenKind::AssignLogicalOr),
    NullCoalescing = static_cast<int>(TokenKind::AssignNullCoalescing),
  };

  enum class BinaryOp
  {
    Add = static_cast<int>(TokenKind::Add),
    Sub = static_cast<int>(TokenKind::Sub),
    Mul = static_cast<int>(TokenKind::Mul),
    Div = static_cast<int>(TokenKind::Div),
    Mod = static_cast<int>(TokenKind::Mod),
    BitwiseAnd = static_cast<int>(TokenKind::BitwiseAnd),
    BitwiseOr = static_cast<int>(TokenKind::BitwiseOr),
    BitwiseXor = static_cast<int>(TokenKind::BitwiseXor),
    Equal = static_cast<int>(TokenKind::Equal),
    NotEqual = static_cast<int>(TokenKind::NotEqual),
    LessThan = static_cast<int>(TokenKind::LessThan),
    GreaterThan = static_cast<int>(TokenKind::GreaterThan),
    LessThanEqual = static_cast<int>(TokenKind::LessThanEqual),
    GreaterThanEqual = static_cast<int>(TokenKind::GreaterThanEqual),
    LeftShift = static_cast<int>(TokenKind::LeftShift),
    RightShift = static_cast<int>(TokenKind::RightShift),
    LogicalAnd = static_cast<int>(TokenKind::LogicalAnd),
    LogicalOr = static_cast<int>(TokenKind::LogicalOr),
    NullCoalescing = static_cast<int>(TokenKind::NullCoalescing),
  };

  enum class UnaryOp
  {
    Not = static_cast<int>(TokenKind::Not),
    Add = static_cast<int>(TokenKind::Add),
    Sub = static_cast<int>(TokenKind::Sub),
    BitwiseNot = static_cast<int>(TokenKind::BitwiseNot),
  };

  std::u32string ToString(TokenKind kind);
  std::u32string ToString(AssignOp op);
  std::u32string ToString(BinaryOp op);
  std::u32string ToString(UnaryOp op);

  class Token final : public Node
  {
  public:
    DEFAULT_COPY_AND_ASSIGN(Token);

    TokenKind kind;
    std::optional<std::u32string> text;

    Token(
      const std::optional<Position>& position = std::nullopt,
      TokenKind kind_ = TokenKind::Eof,
      const std::optional<std::u32string>& text_ = std::nullopt
    )
      : Node(position)
      , kind(kind_)
      , text(text_) {}

    std::u32string ToString() const override;
  };
}
