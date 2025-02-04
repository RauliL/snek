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
#include <string>
#include <unordered_map>
#include <vector>

#include "snek/macros.hpp"

namespace snek::interpreter
{
  struct Parameter;
  class Runtime;
}

namespace snek::interpreter::value
{
  class Base;
  enum class Kind;

  using ptr = std::shared_ptr<Base>;
}

namespace snek::interpreter::type
{
  /**
   * Enumeration of different types supported by the typing system.
   */
  enum class Kind
  {
    Any,
    Boolean,
    Builtin,
    Function,
    Intersection,
    List,
    Record,
    String,
    Tuple,
    Union,
  };

  /**
   * Enumeration of different builtin types included in the interpreter.
   */
  enum class BuiltinKind
  {
    Boolean,
    Float,
    Function,
    Int,
    List,
    Number,
    Record,
    String,
    Void,
  };

  /**
   * Abstract base class for all types.
   */
  class Base
  {
  public:
    DISALLOW_COPY_AND_ASSIGN(Base);

    explicit Base() {}

    virtual Kind kind() const = 0;

    virtual bool Accepts(
      const Runtime& runtime,
      const value::ptr& value
    ) const = 0;

    virtual bool Accepts(
      const std::shared_ptr<Base>& that
    ) const = 0;

    virtual std::u32string ToString() const = 0;
  };

  using ptr = std::shared_ptr<Base>;

  ptr
  Reify(const Runtime& runtime, const std::vector<ptr>& types);

  /**
   * Abstract base class for types that contain multiple types.
   */
  class Multiple : public Base
  {
  public:
    using value_type = ptr;
    using container_type = std::vector<value_type>;

    explicit Multiple(const container_type& types)
      : m_types(types) {}

    inline const container_type& types() const
    {
      return m_types;
    }

  private:
    const container_type m_types;
  };

  /**
   * Type which accepts anything.
   */
  class Any final : public Base
  {
  public:
    explicit Any() {}

    inline Kind kind() const override
    {
      return Kind::Any;
    }

    inline bool Accepts(const Runtime&, const value::ptr&) const override
    {
      return true;
    }

    inline bool Accepts(const ptr&) const override
    {
      return true;
    }

    inline std::u32string ToString() const override
    {
      return U"any";
    }
  };

  class Boolean final : public Base
  {
  public:
    using value_type = bool;

    explicit Boolean(value_type value)
      : m_value(value) {}

    inline Kind kind() const override
    {
      return Kind::Boolean;
    }

    bool Accepts(
      const Runtime& runtime,
      const value::ptr& value
    ) const override;

    bool Accepts(const ptr& that) const override;

    inline std::u32string ToString() const override
    {
      return m_value ? U"true" : U"false";
    }

  private:
    const value_type m_value;
  };

  class Builtin final : public Base
  {
  public:
    explicit Builtin(BuiltinKind kind)
      : m_kind(kind) {}

    inline Kind kind() const override
    {
      return Kind::Builtin;
    }

    inline BuiltinKind builtin_kind() const
    {
      return m_kind;
    }

    bool Accepts(
      const Runtime& runtime,
      const value::ptr& value
    ) const override;

    bool Accepts(const ptr& that) const override;

    std::u32string ToString() const override;

  private:
    const BuiltinKind m_kind;
  };

  class Function final : public Base
  {
  public:
    explicit Function(
      const std::vector<Parameter>& parameters,
      const ptr& return_type = nullptr
    )
      : m_parameters(parameters)
      , m_return_type(return_type) {}

    inline Kind kind() const override
    {
      return Kind::Function;
    }

    inline const std::vector<Parameter>& parameters() const
    {
      return m_parameters;
    }

    inline const ptr& return_type() const
    {
      return m_return_type;
    }

    bool Accepts(
      const Runtime& runtime,
      const value::ptr& value
    ) const override;

    bool Accepts(const ptr& that) const override;

    std::u32string ToString() const override;

  private:
    const std::vector<Parameter> m_parameters;
    const ptr m_return_type;
  };

  class Intersection final : public Multiple
  {
  public:
    explicit Intersection(const container_type& types)
      : Multiple(types) {}

    inline Kind kind() const override
    {
      return Kind::Intersection;
    }

    bool Accepts(
      const Runtime& runtime,
      const value::ptr& value
    ) const override;

    bool Accepts(const ptr& that) const override;

    std::u32string ToString() const override;
  };

  class List final : public Base
  {
  public:
    explicit List(const ptr& element_type)
      : m_element_type(element_type) {}

    inline Kind kind() const override
    {
      return Kind::List;
    }

    inline const ptr& element_type() const
    {
      return m_element_type;
    }

    bool Accepts(
      const Runtime& runtime,
      const value::ptr& value
    ) const override;

    bool Accepts(const ptr& that) const override;

    inline std::u32string ToString() const override
    {
      return m_element_type->ToString() + U"[]";
    }

  private:
    const ptr m_element_type;
  };

  class Record final : public Base
  {
  public:
    using key_type = std::u32string;
    using mapped_type = ptr;
    using container_type = std::unordered_map<key_type, mapped_type>;

    explicit Record(const container_type& fields)
      : m_fields(fields) {}

    inline Kind kind() const override
    {
      return Kind::Record;
    }

    inline const container_type& fields() const
    {
      return m_fields;
    }

    bool Accepts(
      const Runtime& runtime,
      const value::ptr& value
    ) const override;

    bool Accepts(const ptr& that) const override;

    std::u32string ToString() const override;

  private:
    const container_type m_fields;
  };

  class String final : public Base
  {
  public:
    using value_type = std::u32string;
    using const_reference = const value_type&;

    explicit String(const_reference value)
      : m_value(value) {}

    inline Kind kind() const override
    {
      return Kind::String;
    }

    inline const_reference value() const
    {
      return m_value;
    }

    bool Accepts(
      const Runtime& runtime,
      const value::ptr& value
    ) const override;

    bool Accepts(const ptr& that) const override;

    std::u32string ToString() const override;

  private:
    const value_type m_value;
  };

  class Tuple final : public Multiple
  {
  public:
    explicit Tuple(const container_type& types)
      : Multiple(types) {}

    inline Kind kind() const override
    {
      return Kind::Tuple;
    }

    bool Accepts(
      const Runtime& runtime,
      const value::ptr& value
    ) const override;

    bool Accepts(const ptr& that) const override;

    std::u32string ToString() const override;
  };

  class Union final : public Multiple
  {
  public:
    explicit Union(const container_type& types)
      : Multiple(types) {}

    inline Kind kind() const override
    {
      return Kind::Union;
    }

    bool Accepts(
      const Runtime& runtime,
      const value::ptr& value
    ) const override;

    bool Accepts(const ptr& that) const override;

    std::u32string ToString() const override;
  };
}
