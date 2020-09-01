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

#include <peelo/result.hpp>

#include <snek/ast/base.hpp>
#include <snek/error.hpp>

namespace snek
{
  class Interpreter;
  class Scope;
}

namespace snek::value { class Base; }

namespace snek::ast::expr
{
  enum class Kind
  {
    LValue,
    RValue,
  };

  class RValue : public Node
  {
  public:
    using result_type = peelo::result<std::shared_ptr<value::Base>, Error>;
    using bool_result_type = peelo::result<bool, Error>;

    explicit RValue(const Position& position);

    virtual inline Kind kind() const
    {
      return Kind::RValue;
    }

    virtual result_type eval(
      Interpreter& interpreter,
      const Scope& scope
    ) const = 0;

    bool_result_type eval_as_bool(
      Interpreter& interpreter,
      const Scope& scope
    ) const;
  };

  class LValue : public RValue
  {
  public:
    using assign_result_type = std::optional<Error>;

    explicit LValue(const Position& position);

    inline Kind kind() const
    {
      return Kind::LValue;
    }

    virtual assign_result_type assign(
      Interpreter& interpreter,
      Scope& scope,
      const std::shared_ptr<value::Base>& value
    ) const = 0;
  };
}
