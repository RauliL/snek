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
#include <snek/ast/expr/call.hpp>
#include <snek/value/func.hpp>

namespace snek::ast::expr
{
  Call::Call(
    const Position& position,
    const std::shared_ptr<RValue>& callee,
    const std::vector<std::shared_ptr<RValue>>& arguments
  )
    : RValue(position)
    , m_callee(callee)
    , m_arguments(arguments) {}

  RValue::result_type
  Call::eval(Interpreter& interpreter, const Scope& scope) const
  {
    const auto callee = m_callee->eval(interpreter, scope);
    Message::container_type arguments;

    if (!callee)
    {
      return callee;
    }
    else if (callee.value()->kind() != value::Kind::Func)
    {
      return result_type::error({
        m_callee->position(),
        callee.value()->type(interpreter)->to_string() +
        U" is not an function."
      });
    }
    arguments.reserve(m_arguments.size());
    for (const auto& argument : m_arguments)
    {
      const auto result = argument->eval(interpreter, scope);

      if (!result)
      {
        return result;
      }
      arguments.push_back(result.value());
    }

    return std::static_pointer_cast<value::Func>(callee.value())->send(
      interpreter,
      arguments,
      position()
    );
  }
}
