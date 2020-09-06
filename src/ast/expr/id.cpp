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
#include <snek/ast/expr/id.hpp>
#include <snek/scope.hpp>

namespace snek::ast::expr
{
  Id::Id(const Position& position, const std::u32string& name)
    : LValue(position)
    , m_name(name) {}

  RValue::result_type
  Id::eval(Interpreter&, const Scope& scope) const
  {
    if (const auto value = scope.find_variable(m_name))
    {
      return result_type::ok(*value);
    }

    return result_type::error({
      position(),
      U"Unknown variable: `" + m_name + U"'"
    });
  }

  LValue::assign_result_type
  Id::assign(
    Interpreter&,
    Scope& scope,
    const std::shared_ptr<value::Base>& value
  ) const
  {
    if (!scope.add_variable(m_name, value))
    {
      return assign_result_type({
        position(),
        U"Variable `" + m_name + U"' has already been defined."
      });
    }

    return assign_result_type();
  }
}
