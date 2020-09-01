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

#include <functional>
#include <variant>
#include <vector>

#include <peelo/result.hpp>

#include <snek/error.hpp>
#include <snek/parameter.hpp>
#include <snek/scope.hpp>
#include <snek/type/base.hpp>
#include <snek/value/base.hpp>

namespace snek
{
  class Interpreter;
  class Message;
}

namespace snek::ast::stmt
{
  class Block;
}

namespace snek::value
{
  class Func : public Base
  {
  public:
    using result_type = peelo::result<Ptr, Error>;
    using callback_type = std::function<result_type(
      Interpreter&,
      const Message&
    )>;
    using body_type = std::variant<
      callback_type,
      std::shared_ptr<ast::stmt::Block>
    >;

    explicit Func(
      const std::vector<Parameter>& parameters,
      const body_type& body,
      const std::optional<type::Ptr>& return_type = std::nullopt,
      const std::optional<Scope>& enclosing_scope = std::nullopt
    );

    inline Kind kind() const
    {
      return Kind::Func;
    }

    inline const std::vector<Parameter>& parameters() const
    {
      return m_parameters;
    }

    inline const std::optional<type::Ptr>& return_type() const
    {
      return m_return_type;
    }

    inline const body_type& body() const
    {
      return m_body;
    }

    inline const std::optional<Scope>& enclosing_scope() const
    {
      return m_enclosing_scope;
    }

    result_type call(
      Interpreter& interpreter,
      const std::vector<Ptr>& arguments,
      const std::optional<ast::Position>& position = std::nullopt
    ) const;

    std::shared_ptr<type::Base> type(const Interpreter& interpreter) const;

    bool equals(const std::shared_ptr<Base>& that) const;

    std::u32string to_string() const;

  private:
    const std::vector<Parameter> m_parameters;
    const body_type m_body;
    const std::optional<type::Ptr> m_return_type;
    const std::optional<Scope> m_enclosing_scope;
  };

  using FuncPtr = std::shared_ptr<Func>;
}
