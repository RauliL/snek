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
#include <snek/scope.hpp>

namespace snek
{
  Scope::Scope(const std::shared_ptr<Scope>& parent_scope)
    : m_parent_scope(parent_scope) {}

  Scope::Scope(
    const type_container_type& types,
    const variable_container_type& variables
  )
    : m_types(types)
    , m_variables(variables) {}

  Scope::Scope(const Scope& that)
    : m_types(that.m_types)
    , m_variables(that.m_variables)
    , m_parent_scope(that.m_parent_scope) {}

  Scope::Scope(Scope&& that)
    : m_types(std::move(that.m_types))
    , m_variables(std::move(that.m_variables))
    , m_parent_scope(std::move(that.m_parent_scope)) {}

  Scope&
  Scope::operator=(const Scope& that)
  {
    m_types = that.m_types;
    m_variables = that.m_variables;
    m_parent_scope = that.m_parent_scope;

    return *this;
  }

  Scope&
  Scope::operator=(Scope&& that)
  {
    m_types = std::move(that.m_types);
    m_variables = std::move(that.m_variables);
    m_parent_scope = std::move(that.m_parent_scope);

    return *this;
  }

  std::optional<std::shared_ptr<type::Base>>
  Scope::find_type(const std::u32string& name) const
  {
    const auto entry = m_types.find(name);

    if (entry != std::end(m_types))
    {
      return entry->second;
    }

    return m_parent_scope ? m_parent_scope->find_type(name) : std::nullopt;
  }

  std::optional<std::shared_ptr<value::Base>>
  Scope::find_variable(const std::u32string& name) const
  {
    const auto entry = m_variables.find(name);

    if (entry != std::end(m_variables))
    {
      return entry->second;
    }

    return m_parent_scope ? m_parent_scope->find_variable(name) : std::nullopt;
  }

  bool
  Scope::add_type(
    const std::u32string& name,
    const std::shared_ptr<type::Base>& type
  )
  {
    if (m_types.find(name) != std::end(m_types))
    {
      return false;
    }
    m_types[name] = type;

    return true;
  }

  bool
  Scope::add_variable(
    const std::u32string& name,
    const std::shared_ptr<value::Base>& value
  )
  {
    if (m_variables.find(name) != std::end(m_variables))
    {
      return false;
    }
    m_variables[name] = value;

    return true;
  }
}
