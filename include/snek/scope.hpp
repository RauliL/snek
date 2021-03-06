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
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace snek::type { class Base; }
namespace snek::value { class Base; }

namespace snek
{
  class Scope
  {
  public:
    using type_container_type = std::unordered_map<
      std::u32string,
      std::shared_ptr<type::Base>
    >;
    using variable_container_type = std::unordered_map<
      std::u32string,
      std::shared_ptr<value::Base>
    >;

    Scope(const std::shared_ptr<Scope>& parent_scope = nullptr);
    Scope(
      const type_container_type& exported_types,
      const variable_container_type& exported_variables
    );
    Scope(const Scope& that) = default;
    Scope(Scope&& that) = default;
    Scope& operator=(const Scope& that) = default;
    Scope& operator=(Scope&& that) = default;

    inline const type_container_type& types() const
    {
      return m_types;
    }

    inline const type_container_type& exported_types() const
    {
      return m_exported_types;
    }

    inline const variable_container_type &variables() const
    {
      return m_variables;
    }

    inline const variable_container_type& exported_variables() const
    {
      return m_exported_variables;
    }

    inline const std::shared_ptr<Scope>& parent_scope() const
    {
      return m_parent_scope;
    }

    std::optional<std::shared_ptr<type::Base>> find_type(
      const std::u32string& name
    ) const;

    std::optional<std::shared_ptr<type::Base>> find_exported_type(
      const std::u32string& name
    ) const;

    std::optional<std::shared_ptr<value::Base>> find_variable(
      const std::u32string& name
    ) const;

    std::optional<std::shared_ptr<value::Base>> find_exported_variable(
      const std::u32string& name
    ) const;

    bool add_type(
      const std::u32string& name,
      const std::shared_ptr<type::Base>& type
    );

    bool add_exported_type(
      const std::u32string& name,
      const std::shared_ptr<type::Base>& type
    );

    bool add_variable(
      const std::u32string& name,
      const std::shared_ptr<value::Base>& value
    );

    bool add_exported_variable(
      const std::u32string& name,
      const std::shared_ptr<value::Base>& value
    );

  private:
    type_container_type m_types;
    type_container_type m_exported_types;
    variable_container_type m_variables;
    variable_container_type m_exported_variables;
    std::shared_ptr<Scope> m_parent_scope;
  };
}
