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

#include "snek/parser/parameter.hpp"

namespace snek::parser::expression
{
  class Base;

  using ptr = std::shared_ptr<Base>;
}

namespace snek::parser::statement
{
  class Base;

  using ptr = std::shared_ptr<Base>;
}

namespace snek::parser::type
{
  class Base;

  using ptr = std::shared_ptr<Base>;
}

namespace snek::parser::field
{
  enum class Kind
  {
    Computed,
    Function,
    Named,
    Shorthand,
    Spread,
  };

  class Base : public Node
  {
  public:
    DISALLOW_COPY_AND_ASSIGN(Base);

    explicit Base(const std::optional<Position>& position)
      : Node(position) {}

    virtual Kind kind() const = 0;
  };

  using ptr = std::shared_ptr<Base>;

  ptr Parse(Lexer& lexer);

  class Computed final : public Base
  {
  public:
    const expression::ptr key;
    const expression::ptr value;

    explicit Computed(
      const std::optional<Position>& position,
      const expression::ptr& key_,
      const expression::ptr& value_
    )
      : Base(position)
      , key(key_)
      , value(value_) {}

    inline Kind kind() const override
    {
      return Kind::Computed;
    }

    std::u32string ToString() const override;
  };

  class Function final : public Base
  {
  public:
    const std::u32string name;
    const std::vector<Parameter> parameters;
    const type::ptr return_type;
    const statement::ptr body;

    explicit Function(
      const std::optional<Position>& position,
      const std::u32string& name_,
      const std::vector<Parameter>& parameters_,
      const type::ptr& return_type_,
      const statement::ptr& body_
    )
      : Base(position)
      , name(name_)
      , parameters(parameters_)
      , return_type(return_type_)
      , body(body_) {}

    inline Kind kind() const override
    {
      return Kind::Function;
    }

    std::u32string ToString() const override;
  };

  class Named final : public Base
  {
  public:
    const std::u32string name;
    const expression::ptr value;

    explicit Named(
      const std::optional<Position>& position,
      const std::u32string& name_,
      const expression::ptr& value_
    )
      : Base(position)
      , name(name_)
      , value(value_) {}

    inline Kind kind() const override
    {
      return Kind::Named;
    }

    std::u32string ToString() const override;
  };

  class Shorthand final : public Base
  {
  public:
    const std::u32string name;

    explicit Shorthand(
      const std::optional<Position>& position,
      const std::u32string& name_
    )
      : Base(position)
      , name(name_) {}

    inline Kind kind() const override
    {
      return Kind::Shorthand;
    }

    std::u32string ToString() const override;
  };

  class Spread final : public Base
  {
  public:
    const expression::ptr expression;

    explicit Spread(
      const std::optional<Position>& position,
      const expression::ptr& expression_
    )
      : Base(position)
      , expression(expression_) {}

    inline Kind kind() const override
    {
      return Kind::Spread;
    }

    std::u32string ToString() const override;
  };
}
