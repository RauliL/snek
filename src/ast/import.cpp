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
#include <snek/ast/import.hpp>
#include <snek/scope.hpp>
#include <snek/value/record.hpp>

namespace snek::ast::import
{
  Specifier::Specifier(const Position& position)
    : Node(position) {}

  Named::Named(
    const Position& position,
    const std::u32string& name,
    const std::optional<std::u32string>& alias
  )
    : Specifier(position)
    , m_name(name)
    , m_alias(alias) {}

  Specifier::result_type
  Named::import(
    const Scope& module,
    const std::u32string& module_path,
    Scope& scope
  ) const
  {
    if (const auto variable = module.find_variable(m_name, true))
    {
      if (!scope.add_variable(m_alias ? *m_alias : m_name, *variable, false))
      {
        return result_type({
          position(),
          U"Variable `" +
          (m_alias ? *m_alias : m_name) +
          U"' has already been defined."
        });
      }

      return std::nullopt;
    }
    else if (const auto type = module.find_type(m_name, true))
    {
      if (!scope.add_type(m_alias ? *m_alias : m_name, *type, false))
      {
        return result_type({
          position(),
          U"Type `" +
          (m_alias ? *m_alias : m_name) +
          U"' has already been defined."
        });
      }

      return std::nullopt;
    }

    return result_type({
      position(),
      U"Module `" + module_path + U"' has no named export `" + m_name + U"'."
    });
  }

  Star::Star(const Position& position, const std::u32string& name)
    : Specifier(position)
    , m_name(name) {}

  Specifier::result_type
  Star::import(
    const Scope& module,
    const std::u32string& module_path,
    Scope& scope
  ) const
  {
    value::Record::container_type fields;
    std::shared_ptr<value::Record> record;

    for (const auto& variable : module.variables())
    {
      if (variable.second.second)
      {
        fields[variable.first] = variable.second.first;
      }
    }
    record = std::make_shared<value::Record>(fields);
    if (!scope.add_variable(m_name, record, false))
    {
      return result_type({
        position(),
        U"Variable `" + m_name + U" has already been defined."
      });
    }

    return std::nullopt;
  }
}
