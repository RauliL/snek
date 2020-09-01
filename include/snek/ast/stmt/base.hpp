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

#include <snek/ast/base.hpp>
#include <snek/error.hpp>

namespace snek
{
  class Interpreter;
  class Scope;
}

namespace snek::value
{
  class Base;
}

namespace snek::ast::stmt
{
  enum class Jump
  {
    Break,
    Continue,
    Return,
  };

  class ExecContext
  {
  public:
    using jump_type = std::optional<Jump>;
    using error_type = std::optional<Error>;
    using value_type = std::shared_ptr<value::Base>;

    ExecContext();
    ExecContext(const ExecContext& that);
    ExecContext(ExecContext&& that);
    ExecContext& operator=(const ExecContext& that);
    ExecContext& operator=(ExecContext&& that);

    inline jump_type& jump()
    {
      return m_jump;
    }

    inline const jump_type& jump() const
    {
      return m_jump;
    }

    inline error_type& error()
    {
      return m_error;
    }

    inline const error_type& error() const
    {
      return m_error;
    }

    inline value_type& value()
    {
      return m_value;
    }

    inline const value_type& value() const
    {
      return m_value;
    }

  private:
    jump_type m_jump;
    error_type m_error;
    value_type m_value;
  };

  class Base : public Node
  {
  public:
    explicit Base(const Position& position);

    virtual void exec(
      Interpreter& interpreter,
      Scope& scope,
      ExecContext& context
    ) const = 0;
  };
}

namespace snek::ast
{
  using Stmt = std::shared_ptr<stmt::Base>;
}
