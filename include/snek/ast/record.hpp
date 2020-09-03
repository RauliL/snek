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
#include <unordered_map>

#include <snek/ast/base.hpp>
#include <snek/error.hpp>

namespace snek
{
  class Interpreter;
  class Scope;
}

namespace snek::ast::expr { class RValue; }
namespace snek::value { class Base; }

namespace snek::ast::record
{
  class Field : public Node
  {
  public:
    using result_type = std::optional<Error>;
    using record_type = std::unordered_map<
      std::u32string,
      std::shared_ptr<value::Base>
    >;

    explicit Field(const Position& position);

    virtual result_type eval(
      Interpreter& interpreter,
      const Scope& scope,
      record_type& record
    ) const = 0;

    virtual result_type assign(
      Interpreter& interpreter,
      Scope& scope,
      const record_type& record
    ) const;
  };

  class Named final : public Field
  {
  public:
    explicit Named(
      const Position& position,
      const std::u32string& name,
      const std::shared_ptr<expr::RValue>& value_expression
    );

    inline const std::u32string& name() const
    {
      return m_name;
    }

    inline const std::shared_ptr<expr::RValue>& value_expression() const
    {
      return m_value_expression;
    }

    result_type eval(
      Interpreter& interpreter,
      const Scope& scope,
      record_type& record
    ) const;

  private:
    const std::u32string m_name;
    const std::shared_ptr<expr::RValue> m_value_expression;
  };

  class Expr final : public Field
  {
  public:
    explicit Expr(
      const Position& position,
      const std::shared_ptr<expr::RValue>& name_expression,
      const std::shared_ptr<expr::RValue>& value_expression
    );

    inline const std::shared_ptr<expr::RValue>& name_expression() const
    {
      return m_name_expression;
    }

    inline const std::shared_ptr<expr::RValue>& value_expression() const
    {
      return m_value_expression;
    }

    result_type eval(
      Interpreter& interpreter,
      const Scope& scope,
      record_type& record
    ) const;

  private:
    const std::shared_ptr<expr::RValue> m_name_expression;
    const std::shared_ptr<expr::RValue> m_value_expression;
  };

  class Var final : public Field
  {
  public:
    explicit Var(const Position& position, const std::u32string& name);

    inline const std::u32string& name() const
    {
      return m_name;
    }

    result_type eval(
      Interpreter& interpreter,
      const Scope& scope,
      record_type& record
    ) const;

    result_type assign(
      Interpreter& interpreter,
      Scope& scope,
      const record_type& record
    ) const;

  private:
    const std::u32string m_name;
  };

  class Spread final : public Field
  {
  public:
    explicit Spread(
      const Position& position,
      const std::shared_ptr<expr::RValue>& expression
    );

    inline const std::shared_ptr<expr::RValue>& expression() const
    {
      return m_expression;
    }

    result_type eval(
      Interpreter& interpreter,
      const Scope& scope,
      record_type& record
    ) const;

  private:
    const std::shared_ptr<expr::RValue> m_expression;
  };
}
