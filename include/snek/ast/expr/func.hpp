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

#include <vector>

#include <snek/ast/expr/base.hpp>

namespace snek::ast { class Parameter; }
namespace snek::ast::stmt { class Block; }
namespace snek::ast::type { class Base; }

namespace snek::ast::expr
{
  class Func : public RValue
  {
  public:
    explicit Func(
      const Position& position,
      const std::vector<std::shared_ptr<Parameter>>& parameters,
      const std::shared_ptr<ast::stmt::Block>& body,
      const std::optional<std::shared_ptr<ast::type::Base>>& return_type
        = std::nullopt
    );

    inline const std::vector<std::shared_ptr<Parameter>>& parameters() const
    {
      return m_parameters;
    }

    inline const std::shared_ptr<ast::stmt::Block>& body() const
    {
      return m_body;
    }

    inline const std::optional<std::shared_ptr<ast::type::Base>>& return_type()
      const
    {
      return m_return_type;
    }

    result_type eval(Interpreter& interpreter, const Scope& scope) const;

  private:
    const std::vector<std::shared_ptr<Parameter>> m_parameters;
    const std::shared_ptr<stmt::Block> m_body;
    const std::optional<std::shared_ptr<ast::type::Base>> m_return_type;
  };
}