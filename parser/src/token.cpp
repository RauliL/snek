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
#include "snek/parser/token.hpp"

namespace snek::parser
{
  std::u32string
  ToString(TokenKind kind)
  {
    switch (kind)
    {
      case TokenKind::Eof:
        return U"end of input";

      case TokenKind::NewLine:
        return U"line terminator";

      case TokenKind::Indent:
        return U"indent";

      case TokenKind::Dedent:
        return U"dedent";

      case TokenKind::LeftParen:
        return U"`('";

      case TokenKind::RightParen:
        return U"`)'";

      case TokenKind::LeftBracket:
        return U"`['";

      case TokenKind::RightBracket:
        return U"`]'";

      case TokenKind::LeftBrace:
        return U"`{'";

      case TokenKind::RightBrace:
        return U"`}'";

      case TokenKind::Dot:
        return U"`.'";

      case TokenKind::ConditionalDot:
        return U"`?.'";

      case TokenKind::Comma:
        return U"`,'";

      case TokenKind::Colon:
        return U"`:'";

      case TokenKind::Semicolon:
        return U"`;'";

      case TokenKind::Arrow:
        return U"`->'";

      case TokenKind::FatArrow:
        return U"`=>'";

      case TokenKind::Spread:
        return U"`...'";

      case TokenKind::Ternary:
        return U"`?'";

      case TokenKind::Id:
        return U"identifier";

      case TokenKind::Int:
        return U"integer literal";

      case TokenKind::Float:
        return U"float literal";

      case TokenKind::String:
        return U"string literal";

      case TokenKind::Add:
        return U"`+'";

      case TokenKind::Sub:
        return U"`-'";

      case TokenKind::Mul:
        return U"`*'";

      case TokenKind::Div:
        return U"`/'";

      case TokenKind::Mod:
        return U"`%'";

      case TokenKind::Not:
        return U"`!'";

      case TokenKind::BitwiseAnd:
        return U"`&'";

      case TokenKind::BitwiseOr:
        return U"`|'";

      case TokenKind::BitwiseNot:
        return U"`~'";

      case TokenKind::BitwiseXor:
        return U"`^'";

      case TokenKind::Equal:
        return U"`=='";

      case TokenKind::NotEqual:
        return U"`!='";

      case TokenKind::LessThan:
        return U"`<'";

      case TokenKind::GreaterThan:
        return U"`>'";

      case TokenKind::LessThanEqual:
        return U"`<='";

      case TokenKind::GreaterThanEqual:
        return U"`>='";

      case TokenKind::LeftShift:
        return U"`<<'";

      case TokenKind::RightShift:
        return U"`>>'";

      case TokenKind::LogicalAnd:
        return U"`&&'";

      case TokenKind::LogicalOr:
        return U"`||'";

      case TokenKind::Increment:
        return U"`++'";

      case TokenKind::Decrement:
        return U"`--'";

      case TokenKind::NullCoalescing:
        return U"`\?\?'";

      case TokenKind::Assign:
        return U"`='";

      case TokenKind::AssignAdd:
        return U"`+='";

      case TokenKind::AssignSub:
        return U"`-='";

      case TokenKind::AssignMul:
        return U"`*='";

      case TokenKind::AssignDiv:
        return U"`/='";

      case TokenKind::AssignMod:
        return U"`%='";

      case TokenKind::AssignBitwiseAnd:
        return U"`&='";

      case TokenKind::AssignBitwiseOr:
        return U"`|='";

      case TokenKind::AssignBitwiseXor:
        return U"`^='";

      case TokenKind::AssignLeftShift:
        return U"`<<='";

      case TokenKind::AssignRightShift:
        return U"`>>='";

      case TokenKind::AssignLogicalAnd:
        return U"`&&='";

      case TokenKind::AssignLogicalOr:
        return U"`||='";

      case TokenKind::AssignNullCoalescing:
        return U"`\?\?='";

      case TokenKind::KeywordAs:
        return U"`as'";

      case TokenKind::KeywordBreak:
        return U"`break'";

      case TokenKind::KeywordConst:
        return U"`const'";

      case TokenKind::KeywordContinue:
        return U"`continue'";

      case TokenKind::KeywordElse:
        return U"`else'";

      case TokenKind::KeywordExport:
        return U"`export'";

      case TokenKind::KeywordFalse:
        return U"`false'";

      case TokenKind::KeywordFor:
        return U"`for'";

      case TokenKind::KeywordFrom:
        return U"`from'";

      case TokenKind::KeywordIf:
        return U"`if'";

      case TokenKind::KeywordImport:
        return U"`import'";

      case TokenKind::KeywordLet:
        return U"`let'";

      case TokenKind::KeywordNull:
        return U"`null'";

      case TokenKind::KeywordPass:
        return U"`pass'";

      case TokenKind::KeywordReturn:
        return U"`return'";

      case TokenKind::KeywordTrue:
        return U"`true'";

      case TokenKind::KeywordType:
        return U"`type'";

      case TokenKind::KeywordWhile:
        return U"`while'";
    }

    return U"unknown";
  }

  std::u32string
  ToString(AssignOp op)
  {
    switch (op)
    {
      case AssignOp::Add:
        return U"+";

      case AssignOp::Sub:
        return U"-";

      case AssignOp::Mul:
        return U"*";

      case AssignOp::Div:
        return U"/";

      case AssignOp::Mod:
        return U"%";

      case AssignOp::BitwiseAnd:
        return U"&";

      case AssignOp::BitwiseOr:
        return U"|";

      case AssignOp::BitwiseXor:
        return U"^";

      case AssignOp::LeftShift:
        return U"<<";

      case AssignOp::RightShift:
        return U">>";

      case AssignOp::LogicalAnd:
        return U"&&";

      case AssignOp::LogicalOr:
        return U"||";

      case AssignOp::NullCoalescing:
        return U"??";
    }

    return U"unknown";
  }

  std::u32string
  ToString(BinaryOp op)
  {
    switch (op)
    {
      case BinaryOp::Add:
        return U"+";

      case BinaryOp::Sub:
        return U"-";

      case BinaryOp::Mul:
        return U"*";

      case BinaryOp::Div:
        return U"/";

      case BinaryOp::Mod:
        return U"%";

      case BinaryOp::BitwiseAnd:
        return U"&";

      case BinaryOp::BitwiseOr:
        return U"|";

      case BinaryOp::BitwiseXor:
        return U"^";

      case BinaryOp::Equal:
        return U"==";

      case BinaryOp::NotEqual:
        return U"!=";

      case BinaryOp::LessThan:
        return U"<";

      case BinaryOp::GreaterThan:
        return U">";

      case BinaryOp::LessThanEqual:
        return U"<=";

      case BinaryOp::GreaterThanEqual:
        return U">=";

      case BinaryOp::LeftShift:
        return U"<<";

      case BinaryOp::RightShift:
        return U">>";

      case BinaryOp::LogicalAnd:
        return U"&&";

      case BinaryOp::LogicalOr:
        return U"||";

      case BinaryOp::NullCoalescing:
        return U"??";
    }

    return U"unknown";
  }

  std::u32string
  ToString(UnaryOp op)
  {
    switch (op)
    {
      case UnaryOp::Add:
        return U"+";

      case UnaryOp::BitwiseNot:
        return U"~";

      case UnaryOp::Not:
        return U"!";

      case UnaryOp::Sub:
        return U"-";
    }

    return U"unknown";
  }

  std::u32string
  Token::ToString() const
  {
    if (text && kind != TokenKind::String)
    {
      return std::u32string(1, U'`').append(*text).append(1, U'\'');
    }

    return parser::ToString(kind);
  }
}
