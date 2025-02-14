/*
 * Copyright (c) 2020-2025, Rauli Laine
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
#include "snek/interpreter/error.hpp"
#include "snek/interpreter/runtime.hpp"

namespace snek::interpreter
{
  namespace api
  {
    void AddGlobalVariables(const Runtime*, Scope::variable_container_type&);
  }

  Scope::ptr
  Scope::MakeRootScope(const Runtime* runtime)
  {
    auto scope = std::make_shared<Scope>();

    scope->m_types[U"Boolean"] = { runtime->boolean_type() };
    scope->m_types[U"Float"] = { runtime->float_type() };
    scope->m_types[U"Function"] = { runtime->function_type() };
    scope->m_types[U"Int"] = { runtime->int_type() };
    scope->m_types[U"List"] = { runtime->list_type() };
    scope->m_types[U"Number"] = { runtime->number_type() };
    scope->m_types[U"Object"] = { runtime->any_type() };
    scope->m_types[U"Record"] = { runtime->record_type() };
    scope->m_types[U"String"] = { runtime->string_type() };

    scope->m_variables[U"Boolean"] = { runtime->boolean_prototype(), true };
    scope->m_variables[U"Float"] = { runtime->float_prototype(), true };
    scope->m_variables[U"Function"] = { runtime->function_prototype(), true };
    scope->m_variables[U"Int"] = { runtime->int_prototype(), true };
    scope->m_variables[U"Number"] = { runtime->number_prototype(), true };
    scope->m_variables[U"List"] = { runtime->list_prototype(), true };
    scope->m_variables[U"Object"] = { runtime->object_prototype(), true };
    scope->m_variables[U"Record"] = { runtime->record_prototype(), true };
    scope->m_variables[U"String"] = { runtime->string_prototype(), true };

    api::AddGlobalVariables(runtime, scope->m_variables);

    return scope;
  }

  std::vector<std::pair<std::u32string, value::ptr>>
  Scope::GetExportedVariables() const
  {
    std::vector<std::pair<std::u32string, value::ptr>> result;

    for (const auto& variable : m_variables)
    {
      if (variable.second.exported)
      {
        result.push_back(std::make_pair(
          variable.first,
          variable.second.value
        ));
      }
    }

    return result;
  }

  std::vector<std::pair<std::u32string, type::ptr>>
  Scope::GetExportedTypes() const
  {
    std::vector<std::pair<std::u32string, type::ptr>> result;

    for (const auto& type : m_types)
    {
      if (type.second.exported)
      {
        result.push_back(std::make_pair(type.first, type.second.type));
      }
    }

    return result;
  }

  bool
  Scope::FindVariable(
    const std::u32string& name,
    value::ptr& slot,
    bool imported
  ) const
  {
    const auto it = m_variables.find(name);

    if (it != std::end(m_variables) && (!imported || it->second.exported))
    {
      slot = it->second.value;

      return true;
    }

    return m_parent && !imported ? m_parent->FindVariable(name, slot) : false;
  }

  void
  Scope::DeclareVariable(
    const std::u32string& name,
    const value::ptr& value,
    bool read_only,
    bool exported
  )
  {
    const auto it = m_variables.find(name);

    if (it != std::end(m_variables))
    {
      // TODO: Include stack trace.
      throw Error{
        {},
        U"Variable `" +
        name +
        U"' has already been declared."
      };
    }
    m_variables[name] = Variable{ value, read_only, exported };
  }

  void
  Scope::SetVariable(
    const std::u32string& name,
    const value::ptr& value
  )
  {
    auto it = m_variables.find(name);

    if (it != std::end(m_variables))
    {
      if (it->second.read_only)
      {
        // TODO: Include stack trace.
        throw Error{
          {},
          U"Variable `" +
          name +
          U"' has been declared as read only."
        };
      }
      it->second.value = value;
    }
    else if (m_parent)
    {
      m_parent->SetVariable(name, value);
    } else {
      // TODO: Include stack trace.
      throw Error{ {}, U"Unknown variable: `" + name + U"'." };
    }
  }

  bool
  Scope::FindType(
    const std::u32string& name,
    type::ptr& slot,
    bool imported
  ) const
  {
    const auto it = m_types.find(name);

    if (it != std::end(m_types) && (!imported || it->second.exported))
    {
      slot = it->second.type;

      return true;
    }

    return m_parent && !imported ? m_parent->FindType(name, slot) : false;
  }

  void Scope::DeclareType(
    const std::u32string& name,
    const type::ptr& type,
    bool exported
  )
  {
    const auto it = m_types.find(name);

    if (it != std::end(m_types))
    {
      // TODO: Include stack trace.
      throw Error{
        {},
        U"Type `'" +
        name +
        U"' has already been declared."
      };
    }
    m_types[name] = { type, exported };
  }
}
