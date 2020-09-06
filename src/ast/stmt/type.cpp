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
#include <snek/ast/stmt/type.hpp>
#include <snek/ast/type/base.hpp>
#include <snek/scope.hpp>

namespace snek::ast::stmt
{
  Type::Type(
    const Position& position,
    const std::u32string& name,
    const std::shared_ptr<type::Base>& type,
    bool is_export
  )
    : Base(position)
    , m_name(name)
    , m_type(type)
    , m_is_export(is_export) {}

  std::u32string
  Type::to_string() const
  {
    std::u32string result;

    if (m_is_export)
    {
      result += U"export ";
    }
    result += U" type ";
    result += m_name;
    result += U" = ";
    result += m_type->to_string();

    return result;
  }

  void
  Type::exec(
    Interpreter& interpreter,
    Scope& scope,
    ExecContext& context
  ) const
  {
    const auto type = m_type->eval(interpreter, scope);

    if (!type)
    {
      context.error() = type.error();
    }
    else if (!scope.add_type(m_name, type.value(), m_is_export))
    {
      context.error() = {
        position(),
        U"Type `" + m_name + U"' has already been defined."
      };
    }
  }
}
