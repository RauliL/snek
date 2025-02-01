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

#include <memory>

#include "snek/parser/lexer.hpp"

namespace snek::parser::expression
{
  class Base;

  using ptr = std::shared_ptr<Base>;
}

namespace snek::parser::field
{
  enum class Kind
  {
    Computed,
    Named,
    Shorthand,
    Spread,
  };

  class Base : public Node
  {
  public:
    DISALLOW_COPY_AND_ASSIGN(Base);

    explicit Base(const Position& position)
      : Node(position) {}

    virtual Kind kind() const = 0;
  };

  using ptr = std::shared_ptr<Base>;

  ptr Parse(Lexer& lexer);

  class Computed final : public Base
  {
  public:
    explicit Computed(
      const Position& position,
      const expression::ptr& key,
      const expression::ptr& value
    )
      : Base(position)
      , m_key(key)
      , m_value(value) {}

    inline Kind kind() const override
    {
      return Kind::Computed;
    }

    inline const expression::ptr& key() const
    {
      return m_key;
    }

    inline const expression::ptr& value() const
    {
      return m_value;
    }

    std::u32string ToString() const override;

  private:
    const expression::ptr m_key;
    const expression::ptr m_value;
  };

  class Named final : public Base
  {
  public:
    explicit Named(
      const Position& position,
      const std::u32string& name,
      const expression::ptr& value
    )
      : Base(position)
      , m_name(name)
      , m_value(value) {}

    inline Kind kind() const override
    {
      return Kind::Named;
    }

    inline const std::u32string& name() const
    {
      return m_name;
    }

    inline const expression::ptr& value() const
    {
      return m_value;
    }

    std::u32string ToString() const override;

  private:
    const std::u32string m_name;
    const expression::ptr m_value;
  };

  class Shorthand final : public Base
  {
  public:
    explicit Shorthand(
      const Position& position,
      const std::u32string& name
    )
      : Base(position)
      , m_name(name) {}

    inline Kind kind() const override
    {
      return Kind::Shorthand;
    }

    inline const std::u32string& name() const
    {
      return m_name;
    }

    std::u32string ToString() const override;

  private:
    const std::u32string m_name;
  };

  class Spread final : public Base
  {
  public:
    explicit Spread(
      const Position& position,
      const expression::ptr& expression
    )
      : Base(position)
      , m_expression(expression) {}

    inline Kind kind() const override
    {
      return Kind::Spread;
    }

    inline const expression::ptr& expression() const
    {
      return m_expression;
    }

    std::u32string ToString() const override;

  private:
    const expression::ptr m_expression;
  };
}
