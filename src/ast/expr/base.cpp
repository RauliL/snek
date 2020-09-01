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
#include <snek/type/base.hpp>
#include <snek/value/bool.hpp>

namespace snek::ast::expr
{
  RValue::RValue(const Position& position)
    : Node(position) {}

  RValue::bool_result_type
  RValue::eval_as_bool(Interpreter& interpreter, const Scope& scope) const
  {
    const auto result = eval(interpreter, scope);

    if (!result)
    {
      return bool_result_type::error(result.error());
    }
    else if (result.value()->kind() != value::Kind::Bool)
    {
      return bool_result_type::error({
        position(),
        U"Expected Bool, got " +
        result.value()->type(interpreter)->to_string() +
        U" instead."
      });
    }

    return bool_result_type::ok(
      std::static_pointer_cast<value::Bool>(result.value())->value()
    );
  }

  LValue::LValue(const Position& position)
    : RValue(position) {}
}
