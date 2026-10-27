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

#include <unordered_map>

#include "snek/parser/signature.hpp"

namespace snek::parser::type
{
  enum class Kind
  {
    Boolean,
    Function,
    List,
    Multiple,
    Named,
    Null,
    Record,
    String,
  };

  class Base : public TypedNode<Kind>
  {
  public:
    DISALLOW_COPY_AND_ASSIGN(Base);

    explicit Base(const std::optional<Position>& position, Kind kind)
      : TypedNode(position, kind) {}
  };

  ptr Parse(Lexer& lexer);

  class Boolean final : public Base
  {
  public:
    using value_type = bool;

    const value_type value;

    explicit Boolean(
      const std::optional<Position>& position,
      value_type value_
    )
      : Base(position, Kind::Boolean)
      , value(value_) {}

    inline std::u32string ToString() const override
    {
      return value ? U"true" : U"false";
    }
  };

  class Function final : public Base
  {
  public:
    const Signature signature;

    explicit Function(
      const std::optional<Position>& position,
      const Signature& signature_
    )
      : Base(position, Kind::Function)
      , signature(signature_) {}

    inline std::u32string ToString() const override
    {
      return signature.ToString(U" => ", true);
    }
  };

  class List final : public Base
  {
  public:
    const ptr element_type;

    explicit List(
      const std::optional<Position>& position,
      const ptr& element_type_
    )
      : Base(position, Kind::List)
      , element_type(element_type_) {}

    inline std::u32string ToString() const override
    {
      return element_type->ToString() + U"[]";
    }
  };

  class Multiple final : public Base
  {
  public:
    using value_type = ptr;
    using container_type = std::vector<value_type>;

    enum class MultipleKind
    {
      Intersection,
      Tuple,
      Union,
    };

    const MultipleKind multiple_kind;
    const container_type types;

    explicit Multiple(
      const std::optional<Position>& position,
      MultipleKind multiple_kind_,
      const container_type& types_
    )
      : Base(position, Kind::Multiple)
      , multiple_kind(multiple_kind_)
      , types(types_) {}

    std::u32string ToString() const override;
  };

  class Named final : public Base
  {
  public:
    const std::u32string name;

    explicit Named(
      const std::optional<Position>& position,
      const std::u32string& name_
    )
      : Base(position, Kind::Named)
      , name(name_) {}

    inline std::u32string ToString() const override
    {
      return name;
    }
  };

  class Null final : public Base
  {
  public:
    explicit Null(const std::optional<Position>& position)
      : Base(position, Kind::Null) {}

    inline std::u32string ToString() const override
    {
      return U"null";
    }
  };

  class Record final : public Base
  {
  public:
    using key_type = std::u32string;
    using mapped_type = ptr;
    using container_type = std::unordered_map<key_type, mapped_type>;
    using value_type = container_type::value_type;

    const container_type fields;

    explicit Record(
      const std::optional<Position>& position,
      const container_type& fields_
    )
      : Base(position, Kind::Record)
      , fields(fields_) {}

    std::u32string ToString() const override;
  };

  class String final : public Base
  {
  public:
    using value_type = std::u32string;
    using const_reference = const value_type&;

    const value_type value;

    explicit String(
      const std::optional<Position>& position,
      const_reference value_
    )
      : Base(position, Kind::String)
      , value(value_) {}

    std::u32string ToString() const override;
  };
}
