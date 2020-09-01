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

#include <snek/ast/base.hpp>
#include <snek/error.hpp>

namespace snek { class Scope; }

namespace snek::ast::import
{
  class Specifier : public Node
  {
  public:
    using result_type = std::optional<Error>;

    explicit Specifier(const Position& position);

    virtual result_type import(
      const Scope& module,
      const std::u32string& module_path,
      Scope& scope
    ) const = 0;
  };

  class Named : public Specifier
  {
  public:
    explicit Named(
      const Position& position,
      const std::u32string& name,
      const std::optional<std::u32string>& alias = std::nullopt
    );

    inline const std::u32string& name() const
    {
      return m_name;
    }

    inline const std::optional<std::u32string>& alias() const
    {
      return m_alias;
    }

    result_type import(
      const Scope& module,
      const std::u32string& module_path,
      Scope& scope
    ) const;

  private:
    const std::u32string m_name;
    const std::optional<std::u32string> m_alias;
  };

  class Star : public Specifier
  {
  public:
    explicit Star(
      const Position& position,
      const std::u32string& name
    );

    inline const std::u32string name() const
    {
      return m_name;
    }

    result_type import(
      const Scope& module,
      const std::u32string& module_path,
      Scope& scope
    ) const;

  private:
    const std::u32string m_name;
  };
}
