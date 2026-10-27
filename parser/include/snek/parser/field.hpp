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

#include "snek/parser/signature.hpp"

namespace snek::parser
{
  enum class FieldKind
  {
    Computed,
    Function,
    Named,
    Shorthand,
    Spread,
  };

  class Field : public TypedNode<FieldKind>
  {
  public:
    DISALLOW_COPY_AND_ASSIGN(Field);

    class Computed;
    class Function;
    class Named;
    class Shorthand;
    class Spread;

    explicit Field(const std::optional<Position>& position, FieldKind kind)
      : TypedNode(position, kind) {}

    static field_ptr Parse(Lexer& lexer);
  };

  class Field::Computed final : public Field
  {
  public:
    const expression_ptr key;
    const expression_ptr value;

    explicit Computed(
      const std::optional<Position>& position,
      const expression_ptr& key_,
      const expression_ptr& value_
    )
      : Field(position, FieldKind::Computed)
      , key(key_)
      , value(value_) {}

    std::u32string ToString() const override;
  };

  class Field::Function final : public Field
  {
  public:
    const std::u32string name;
    const Signature signature;
    const statement_ptr body;

    explicit Function(
      const std::optional<Position>& position,
      const std::u32string& name_,
      const Signature& signature_,
      const statement_ptr& body_
    )
      : Field(position, FieldKind::Function)
      , name(name_)
      , signature(signature_)
      , body(body_) {}

    inline std::u32string ToString() const override
    {
      return name + signature.ToString(U" -> ");
    }
  };

  class Field::Named final : public Field
  {
  public:
    const std::u32string name;
    const expression_ptr value;

    explicit Named(
      const std::optional<Position>& position,
      const std::u32string& name_,
      const expression_ptr& value_
    )
      : Field(position, FieldKind::Named)
      , name(name_)
      , value(value_) {}

    std::u32string ToString() const override;
  };

  class Field::Shorthand final : public Field
  {
  public:
    const std::u32string name;

    explicit Shorthand(
      const std::optional<Position>& position,
      const std::u32string& name_
    )
      : Field(position, FieldKind::Shorthand)
      , name(name_) {}

    std::u32string ToString() const override;
  };

  class Field::Spread final : public Field
  {
  public:
    const expression_ptr expression;

    explicit Spread(
      const std::optional<Position>& position,
      const expression_ptr& expression_
    )
      : Field(position, FieldKind::Spread)
      , expression(expression_) {}

    std::u32string ToString() const override;
  };
}
