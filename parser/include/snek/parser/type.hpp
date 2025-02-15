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

#include "snek/parser/parameter.hpp"

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

  class Boolean final : public Base
  {
  public:
    using value_type = bool;

    const value_type value;

    explicit Boolean(
      const std::optional<Position>& position,
      value_type value_
    )
      : Base(position)
      , value(value_) {}

    inline Kind kind() const override
    {
      return Kind::Boolean;
    }

    inline std::u32string ToString() const override
    {
      return value ? U"true" : U"false";
    }
  };

  class Function final : public Base
  {
  public:
    const std::vector<Parameter> parameters;
    const ptr return_type;

    explicit Function(
      const std::optional<Position>& position,
      const std::vector<Parameter>& parameters_,
      const ptr& return_type_
    )
      : Base(position)
      , parameters(parameters_)
      , return_type(return_type_) {}

    inline Kind kind() const override
    {
      return Kind::Function;
    }

    std::u32string ToString() const override;
  };

  class List final : public Base
  {
  public:
    const ptr element_type;

    explicit List(
      const std::optional<Position>& position,
      const ptr& element_type_
    )
      : Base(position)
      , element_type(element_type_) {}

    inline Kind kind() const override
    {
      return Kind::List;
    }

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
      : Base(position)
      , multiple_kind(multiple_kind_)
      , types(types_) {}

    inline Kind kind() const override
    {
      return Kind::Multiple;
    }

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
      : Base(position)
      , name(name_) {}

    inline Kind kind() const override
    {
      return Kind::Named;
    }

    inline std::u32string ToString() const override
    {
      return name;
    }
  };

  class Null final : public Base
  {
  public:
    explicit Null(const std::optional<Position>& position)
      : Base(position) {}

    inline Kind kind() const override
    {
      return Kind::Null;
    }

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

    const container_type fields;

    explicit Record(
      const std::optional<Position>& position,
      const container_type& fields_
    )
      : Base(position)
      , fields(fields_) {}

    inline Kind kind() const override
    {
      return Kind::Record;
    }

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
      : Base(position)
      , value(value_) {}

    inline Kind kind() const override
    {
      return Kind::String;
    }

    std::u32string ToString() const override;
  };
}
