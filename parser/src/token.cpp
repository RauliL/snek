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
  Token::ToString(Token::Kind kind)
  {
    switch (kind)
    {
      case Token::Kind::Eof:
        return U"end of input";

      case Token::Kind::NewLine:
        return U"line terminator";

      case Token::Kind::Indent:
      case Token::Kind::Dedent:
        return U"indentation";

      case Token::Kind::LeftParen:
        return U"`('";

      case Token::Kind::RightParen:
        return U"`)'";

      case Token::Kind::LeftBracket:
        return U"`['";

      case Token::Kind::RightBracket:
        return U"`]'";

      case Token::Kind::LeftBrace:
        return U"`{'";

      case Token::Kind::RightBrace:
        return U"`}'";

      case Token::Kind::Dot:
        return U"`.'";

      case Token::Kind::ConditionalDot:
        return U"`?.'";

      case Token::Kind::Comma:
        return U"`,'";

      case Token::Kind::Colon:
        return U"`:'";

      case Token::Kind::Semicolon:
        return U"`;'";

      case Token::Kind::Arrow:
        return U"`->'";

      case Token::Kind::FatArrow:
        return U"`=>'";

      case Token::Kind::Spread:
        return U"`...'";

      case Token::Kind::Ternary:
        return U"`?'";

      case Token::Kind::Id:
        return U"identifier";

      case Token::Kind::Int:
        return U"integer literal";

      case Token::Kind::Float:
        return U"float literal";

      case Token::Kind::String:
        return U"string literal";

      case Token::Kind::Add:
        return U"`+'";

      case Token::Kind::Sub:
        return U"`-'";

      case Token::Kind::Mul:
        return U"`*'";

      case Token::Kind::Div:
        return U"`/'";

      case Token::Kind::Mod:
        return U"`%'";

      case Token::Kind::Not:
        return U"`!'";

      case Token::Kind::BitwiseAnd:
        return U"`&'";

      case Token::Kind::BitwiseOr:
        return U"`|'";

      case Token::Kind::BitwiseNot:
        return U"`~'";

      case Token::Kind::BitwiseXor:
        return U"`^'";

      case Token::Kind::Equal:
        return U"`=='";

      case Token::Kind::NotEqual:
        return U"`!='";

      case Token::Kind::LessThan:
        return U"`<'";

      case Token::Kind::GreaterThan:
        return U"`>'";

      case Token::Kind::LessThanEqual:
        return U"`<='";

      case Token::Kind::GreaterThanEqual:
        return U"`>='";

      case Token::Kind::LeftShift:
        return U"`<<'";

      case Token::Kind::RightShift:
        return U"`>>'";

      case Token::Kind::LogicalAnd:
        return U"`&&'";

      case Token::Kind::LogicalOr:
        return U"`||'";

      case Token::Kind::Assign:
        return U"`='";

      case Token::Kind::AssignAdd:
        return U"`+='";

      case Token::Kind::AssignSub:
        return U"`-='";

      case Token::Kind::AssignMul:
        return U"`*='";

      case Token::Kind::AssignDiv:
        return U"`/='";

      case Token::Kind::AssignMod:
        return U"`%='";

      case Token::Kind::AssignBitwiseAnd:
        return U"`&='";

      case Token::Kind::AssignBitwiseOr:
        return U"`|='";

      case Token::Kind::AssignBitwiseXor:
        return U"`^='";

      case Token::Kind::AssignLeftShift:
        return U"`<<='";

      case Token::Kind::AssignRightShift:
        return U"`>>='";

      case Token::Kind::KeywordAs:
        return U"`as'";

      case Token::Kind::KeywordBreak:
        return U"`break'";

      case Token::Kind::KeywordConst:
        return U"`const'";

      case Token::Kind::KeywordContinue:
        return U"`continue'";

      case Token::Kind::KeywordElse:
        return U"`else'";

      case Token::Kind::KeywordExport:
        return U"`export'";

      case Token::Kind::KeywordFalse:
        return U"`false'";

      case Token::Kind::KeywordFor:
        return U"`for'";

      case Token::Kind::KeywordIf:
        return U"`if'";

      case Token::Kind::KeywordImport:
        return U"`import'";

      case Token::Kind::KeywordLet:
        return U"`let'";

      case Token::Kind::KeywordNull:
        return U"`null'";

      case Token::Kind::KeywordPass:
        return U"`pass'";

      case Token::Kind::KeywordReturn:
        return U"`return'";

      case Token::Kind::KeywordTrue:
        return U"`true'";

      case Token::Kind::KeywordType:
        return U"`type'";

      case Token::Kind::KeywordWhile:
        return U"`while'";
    }

    return U"unknown";
  }

  std::u32string
  Token::ToString() const
  {
    if (m_text && m_kind != Kind::String)
    {
      return std::u32string(1, U'`').append(*m_text).append(1, U'\'');
    }

    return ToString(m_kind);
  }
}
