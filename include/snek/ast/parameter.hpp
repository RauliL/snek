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

#include <peelo/result.hpp>

#include <snek/ast/base.hpp>
#include <snek/error.hpp>

namespace snek
{
  class Interpreter;
  class Parameter;
  class Scope;
}

namespace snek::ast::type
{
  class Base;
}

namespace snek::ast
{
  class Parameter final : public Node
  {
  public:
    using result_type = peelo::result<snek::Parameter, Error>;

    explicit Parameter(
      const Position& position,
      const std::u32string& name,
      const std::optional<std::shared_ptr<type::Base>>& type
    );

    inline const std::u32string& name() const
    {
      return m_name;
    }

    inline const std::optional<std::shared_ptr<type::Base>>& type() const
    {
      return m_type;
    }

    std::u32string to_string() const;

    result_type eval(const Interpreter& interpreter, const Scope& scope) const;

  private:
    const std::u32string m_name;
    const std::optional<std::shared_ptr<type::Base>> m_type;
  };
}
