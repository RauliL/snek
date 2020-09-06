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
#include <snek/ast/parameter.hpp>
#include <snek/ast/type/base.hpp>
#include <snek/interpreter.hpp>
#include <snek/parameter.hpp>

namespace snek::ast
{
  Parameter::Parameter(
    const Position& position,
    const std::u32string& name,
    const std::optional<std::shared_ptr<type::Base>>& type
  )
    : Node(position)
    , m_name(name)
    , m_type(type) {}

  std::u32string
  Parameter::to_string() const
  {
    std::u32string result;

    result += m_name;
    result += U": ";
    if (m_type)
    {
      result += (*m_type)->to_string();
    } else {
      result += U"Any";
    }

    return result;
  }

  Parameter::result_type
  Parameter::eval(const Interpreter& interpreter, const Scope& scope) const
  {
    std::shared_ptr<snek::type::Base> type;

    if (m_type)
    {
      const auto result = (*m_type)->eval(interpreter, scope);

      if (!result)
      {
        return result_type::error(result.error());
      }
      type = result.value();
    } else {
      type = interpreter.any_type();
    }

    return result_type::ok(snek::Parameter(m_name, type));
  }
}
