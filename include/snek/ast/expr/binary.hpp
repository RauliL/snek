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
#pragma once

#include <snek/ast/expr/base.hpp>

namespace snek::ast::expr
{
  enum class BinaryOperator
  {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Eq,
    Ne,
    Lt,
    Gt,
    Lte,
    Gte,
  };

  class Binary final : public RValue
  {
  public:
    explicit Binary(
      const Position& position,
      const std::shared_ptr<RValue>& left_expression,
      BinaryOperator binary_operator,
      const std::shared_ptr<RValue>& right_expression
    );

    inline const std::shared_ptr<RValue>& left_expression() const
    {
      return m_left_expression;
    }

    inline BinaryOperator binary_operator() const
    {
      return m_binary_operator;
    }

    inline const std::shared_ptr<RValue>& right_expression() const
    {
      return m_right_expression;
    }

    result_type eval(Interpreter& interpreter, const Scope& scope) const;

  private:
    const std::shared_ptr<RValue> m_left_expression;
    BinaryOperator m_binary_operator;
    const std::shared_ptr<RValue> m_right_expression;
  };
}
