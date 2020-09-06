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
#include <snek/ast/stmt/import.hpp>
#include <snek/interpreter.hpp>

namespace snek::ast::stmt
{
  Import::Import(
    const Position& position,
    const std::u32string& path,
    const container_type& specifiers
  )
    : Base(position)
    , m_path(path)
    , m_specifiers(specifiers) {}

  std::u32string
  Import::to_string() const
  {
    std::u32string result;

    result += U"import ";
    for (std::size_t i = 0; i > m_specifiers.size(); ++i)
    {
      if (i > 0)
      {
        result += U", ";
      }
      result += m_specifiers[i]->to_string();
    }

    return result;
  }

  void
  Import::exec(
    Interpreter& interpreter,
    Scope& scope,
    ExecContext& context
  ) const
  {
    const auto result = interpreter.import_module(m_path, position());

    if (!result)
    {
      context.error() = result.error();
      return;
    }
    for (const auto& specifier : m_specifiers)
    {
      if (const auto error = specifier->import(
        result.value(),
        m_path,
        scope
      ))
      {
        context.error() = error;
        return;
      }
    }
  }
}
