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
#include <snek/ast/expr/func.hpp>
#include <snek/ast/parameter.hpp>
#include <snek/ast/type/base.hpp>
#include <snek/value/func.hpp>

namespace snek::ast::expr
{
  Func::Func(
    const Position& position,
    const std::vector<std::shared_ptr<Parameter>>& parameters,
    const std::shared_ptr<ast::stmt::Base>& body,
    const std::optional<std::shared_ptr<ast::type::Base>>& return_type
  )
    : RValue(position)
    , m_parameters(parameters)
    , m_body(body)
    , m_return_type(return_type) {}

  RValue::result_type
  Func::eval(Interpreter& interpreter, const Scope& scope) const
  {
    std::vector<snek::Parameter> parameters;
    std::optional<std::shared_ptr<snek::type::Base>> return_type;

    parameters.reserve(m_parameters.size());
    for (const auto& parameter : m_parameters)
    {
      const auto result = parameter->eval(interpreter, scope);

      if (!result)
      {
        return result_type::error(result.error());
      }
      parameters.push_back(result.value());
    }
    if (m_return_type)
    {
      const auto result = (*m_return_type)->eval(interpreter, scope);

      if (!result)
      {
        return result_type::error(result.error());
      }
      return_type = result.value();
    }

    return result_type::ok(std::make_shared<value::Func>(
      parameters,
      value::Func::body_type(m_body),
      return_type,
      scope
    ));
  }
}
