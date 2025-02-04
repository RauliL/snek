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
#include <unordered_map>

#include "snek/parser/lexer.hpp"

namespace snek::parser::parameter
{
  class Base;

  using ptr = std::shared_ptr<Base>;
}

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

    explicit Base(const Position& position)
      : Node(position) {}

    virtual Kind kind() const = 0;
  };

  using ptr = std::shared_ptr<Base>;

  ptr Parse(Lexer& lexer);

  class Boolean final : public Base
  {
  public:
    using value_type = bool;

    explicit Boolean(const Position& position, value_type value)
      : Base(position)
      , m_value(value) {}

    inline Kind kind() const override
    {
      return Kind::Boolean;
    }

    inline value_type value() const
    {
      return m_value;
    }

    inline std::u32string ToString() const override
    {
      return m_value ? U"true" : U"false";
    }

  private:
    const value_type m_value;
  };

  class Function final : public Base
  {
  public:
    explicit Function(
      const Position& position,
      const std::vector<parameter::ptr>& parameters,
      const ptr& return_type
    )
      : Base(position)
      , m_parameters(parameters)
      , m_return_type(return_type) {}

    inline Kind kind() const override
    {
      return Kind::Function;
    }

    inline const std::vector<parameter::ptr>& parameters() const
    {
      return m_parameters;
    }

    inline const ptr& return_type() const
    {
      return m_return_type;
    }

    std::u32string ToString() const override;

  private:
    const std::vector<parameter::ptr> m_parameters;
    const ptr m_return_type;
  };

  class List final : public Base
  {
  public:
    explicit List(const Position& position, const ptr& element_type)
      : Base(position)
      , m_element_type(element_type) {}

    inline Kind kind() const override
    {
      return Kind::List;
    }

    inline const ptr& element_type() const
    {
      return m_element_type;
    }

    inline std::u32string ToString() const override
    {
      return m_element_type->ToString() + U"[]";
    }

  private:
    const ptr m_element_type;
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

    explicit Multiple(
      const Position& position,
      MultipleKind multiple_kind,
      const container_type& types
    )
      : Base(position)
      , m_multiple_kind(multiple_kind)
      , m_types(types) {}

    inline Kind kind() const override
    {
      return Kind::Multiple;
    }

    inline MultipleKind multiple_kind() const
    {
      return m_multiple_kind;
    }

    inline const container_type& types() const
    {
      return m_types;
    }

    std::u32string ToString() const override;

  private:
    const MultipleKind m_multiple_kind;
    const container_type m_types;
  };

  class Named final : public Base
  {
  public:
    explicit Named(const Position& position, const std::u32string& name)
      : Base(position)
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
      return m_name;
    }

  private:
    const std::u32string m_name;
  };

  class Null final : public Base
  {
  public:
    explicit Null(const Position& position)
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

    explicit Record(const Position& position, const container_type& fields)
      : Base(position)
      , m_fields(fields) {}

    inline Kind kind() const override
    {
      return Kind::Record;
    }

    inline const container_type& fields() const
    {
      return m_fields;
    }

    std::u32string ToString() const override;

  private:
    const container_type m_fields;
  };

  class String final : public Base
  {
  public:
    using value_type = std::u32string;
    using const_reference = const value_type&;

    explicit String(const Position& position, const_reference value)
      : Base(position)
      , m_value(value) {}

    inline Kind kind() const override
    {
      return Kind::String;
    }

    inline const_reference value() const
    {
      return m_value;
    }

    std::u32string ToString() const override;

  private:
    const value_type m_value;
  };
}
