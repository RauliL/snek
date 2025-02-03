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

namespace snek::parser::import
{
  enum class Kind
  {
    Named,
    Star,
  };

  class Base : public Node
  {
  public:
    DISALLOW_COPY_AND_ASSIGN(Base);

    explicit Base(
      const Position& position,
      const std::optional<std::u32string>& alias = std::nullopt
    )
      : Node(position)
      , m_alias(alias) {}

    virtual Kind kind() const = 0;

    inline const std::optional<std::u32string>& alias() const
    {
      return m_alias;
    }

  private:
    const std::optional<std::u32string> m_alias;
  };

  using ptr = std::shared_ptr<Base>;

  ptr ParseSpecifier(Lexer& lexer);

  class Named final : public Base
  {
  public:
    explicit Named(
      const Position& position,
      const std::u32string name,
      const std::optional<std::u32string>& alias = std::nullopt
    )
      : Base(position, alias)
      , m_name(name) {}

    inline Kind kind() const override
    {
      return Kind::Named;
    }

    inline const std::u32string& name() const
    {
      return m_name;
    }

    inline std::u32string ToString() const override
    {
      std::u32string result(name());

      if (const auto al = alias())
      {
        result.append(U" as ").append(*al);
      }

      return result;
    }

  private:
    const std::u32string m_name;
  };

  class Star final : public Base
  {
  public:
    explicit Star(
      const Position& position,
      const std::optional<std::u32string>& alias = std::nullopt
    )
      : Base(position, alias) {}

    inline Kind kind() const override
    {
      return Kind::Star;
    }

    inline std::u32string ToString() const override
    {
      std::u32string result(U"*");

      if (const auto al = alias())
      {
        result.append(U" as ").append(*al);
      }

      return result;
    }
  };
}
