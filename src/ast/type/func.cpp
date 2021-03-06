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
#include <snek/ast/type/func.hpp>
#include <snek/interpreter.hpp>
#include <snek/type/func.hpp>

namespace snek::ast::type
{
  Func::Func(
    const Position& position,
    const std::vector<std::shared_ptr<Parameter>>& parameters,
    const std::optional<std::shared_ptr<Base>>& return_type
  )
    : Base(position)
    , m_parameters(parameters)
    , m_return_type(return_type) {}

  std::u32string
  Func::to_string() const
  {
    std::u32string result;

    result += U'(';
    for (std::size_t i = 0; i < m_parameters.size(); ++i)
    {
      if (i > 0)
      {
        result += U", ";
      }
      result += m_parameters[i]->to_string();
    }
    result += U')';
    if (m_return_type)
    {
      result += U" -> ";
      result += (*m_return_type)->to_string();
    }

    return result;
  }

  Base::result_type
  Func::eval(const Interpreter& interpreter, const Scope& scope) const
  {
    std::vector<snek::Parameter> parameters;
    std::shared_ptr<snek::type::Base> return_type;

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
        return result;
      }
      return_type = result.value();
    } else {
      return_type = interpreter.any_type();
    }

    return result_type::ok(std::make_shared<snek::type::Func>(
      parameters,
      return_type
    ));
  }
}
