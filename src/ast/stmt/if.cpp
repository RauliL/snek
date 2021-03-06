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
#include <snek/ast/expr/base.hpp>
#include <snek/ast/stmt/if.hpp>
#include <snek/interpreter.hpp>

namespace snek::ast::stmt
{
  If::If(
    const Position& position,
    const std::shared_ptr<expr::RValue>& condition,
    const Stmt& then_statement,
    const Stmt& else_statement
  )
    : Base(position)
    , m_condition(condition)
    , m_then_statement(then_statement)
    , m_else_statement(else_statement) {}

  std::u32string
  If::to_string() const
  {
    return (
      U"if " +
      m_condition->to_string() +
      U": " +
      m_then_statement->to_string()
    );
  }

  void
  If::exec(
    Interpreter& interpreter,
    Scope& scope,
    ExecContext& context
  ) const
  {
    const auto condition = m_condition->eval_as_bool(interpreter, scope);

    if (!condition)
    {
      context.error() = condition.error();
    }
    else if (condition.value())
    {
      m_then_statement->exec(interpreter, scope, context);
    }
    else if (m_else_statement)
    {
      m_else_statement->exec(interpreter, scope, context);
    }
  }
}
