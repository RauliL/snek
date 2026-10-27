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

namespace snek::parser
{
  enum class TypeKind
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

  class Type : public TypedNode<TypeKind>
  {
  public:
    DISALLOW_COPY_AND_ASSIGN(Type);

    class Boolean;
    class Function;
    class List;
    class Multiple;
    class Named;
    class Null;
    class Record;
    class String;

    explicit Type(const std::optional<Position>& position, TypeKind kind)
      : TypedNode(position, kind) {}

    static type_ptr Parse(Lexer& lexer);
  };

  class Type::Boolean final : public Type
  {
  public:
    using value_type = bool;

    const value_type value;

    explicit Boolean(
      const std::optional<Position>& position,
      value_type value_
    )
      : Type(position, TypeKind::Boolean)
      , value(value_) {}

    inline std::u32string ToString() const override
    {
      return value ? U"true" : U"false";
    }
  };

  class Type::Function final : public Type
  {
  public:
    const Signature signature;

    explicit Function(
      const std::optional<Position>& position,
      const Signature& signature_
    )
      : Type(position, TypeKind::Function)
      , signature(signature_) {}

    inline std::u32string ToString() const override
    {
      return signature.ToString(U" => ", true);
    }
  };

  class Type::List final : public Type
  {
  public:
    const type_ptr element_type;

    explicit List(
      const std::optional<Position>& position,
      const type_ptr& element_type_
    )
      : Type(position, TypeKind::List)
      , element_type(element_type_) {}

    inline std::u32string ToString() const override
    {
      return element_type->ToString() + U"[]";
    }
  };

  class Type::Multiple final : public Type
  {
  public:
    using value_type = type_ptr;
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
      : Type(position, TypeKind::Multiple)
      , multiple_kind(multiple_kind_)
      , types(types_) {}

    inline bool Is(MultipleKind multiple_kind_) const
    {
      return multiple_kind == multiple_kind_;
    }

    std::u32string ToString() const override;
  };

  class Type::Named final : public Type
  {
  public:
    const std::u32string name;

    explicit Named(
      const std::optional<Position>& position,
      const std::u32string& name_
    )
      : Type(position, TypeKind::Named)
      , name(name_) {}

    inline std::u32string ToString() const override
    {
      return name;
    }
  };

  class Type::Null final : public Type
  {
  public:
    explicit Null(const std::optional<Position>& position)
      : Type(position, TypeKind::Null) {}

    inline std::u32string ToString() const override
    {
      return U"null";
    }
  };

  class Type::Record final : public Type
  {
  public:
    using key_type = std::u32string;
    using mapped_type = type_ptr;
    using container_type = std::unordered_map<key_type, mapped_type>;
    using value_type = container_type::value_type;

    const container_type fields;

    explicit Record(
      const std::optional<Position>& position,
      const container_type& fields_
    )
      : Type(position, TypeKind::Record)
      , fields(fields_) {}

    std::u32string ToString() const override;
  };

  class Type::String final : public Type
  {
  public:
    using value_type = std::u32string;
    using const_reference = const value_type&;

    const value_type value;

    explicit String(
      const std::optional<Position>& position,
      const_reference value_
    )
      : Type(position, TypeKind::String)
      , value(value_) {}

    std::u32string ToString() const override;
  };
}
