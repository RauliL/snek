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
#pragma once

#include "snek/interpreter/type.hpp"
#include "snek/interpreter/value.hpp"

namespace snek::interpreter
{
  class Scope
  {
  public:
    DEFAULT_COPY_AND_ASSIGN(Scope);

    struct Variable
    {
      value::ptr value;
      bool read_only;
      bool exported;
    };

    struct TypeDefinition
    {
      type::ptr type;
      bool exported;
    };

    using ptr = std::shared_ptr<Scope>;
    using variable_container_type = std::unordered_map<
      std::u32string,
      Variable
    >;
    using type_container_type = std::unordered_map<
      std::u32string,
      TypeDefinition
    >;

    static ptr MakeRootScope(const Runtime* runtime);

    explicit Scope(const ptr& parent = nullptr)
      : m_parent(parent) {}

    std::vector<std::pair<std::u32string, value::ptr>>
    GetExportedVariables() const;

    std::vector<std::pair<std::u32string, type::ptr>>
    GetExportedTypes() const;

    bool FindVariable(
      const std::u32string& name,
      value::ptr& slot,
      bool imported = false
    ) const;

    void DeclareVariable(
      const std::u32string& name,
      const value::ptr& value,
      bool read_only = false,
      bool exported = false
    );

    void SetVariable(
      const std::u32string& name,
      const value::ptr& value
    );

    bool FindType(
      const std::u32string& name,
      type::ptr& slot,
      bool imported = false
    ) const;

    void DeclareType(
      const std::u32string& name,
      const type::ptr& type,
      bool exported = false
    );

  private:
    ptr m_parent;
    variable_container_type m_variables;
    type_container_type m_types;
  };
}
