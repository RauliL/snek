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

#include <functional>

#include "snek/interpreter/config.hpp"
#include "snek/interpreter/parameter.hpp"
#include "snek/parser/parameter.hpp"
#include "snek/parser/statement.hpp"

namespace snek::interpreter
{
  class Runtime;
  class Scope;
}

namespace snek::interpreter::value
{
  enum class Kind
  {
    Boolean,
    Float,
    Function,
    Int,
    List,
    Null,
    Record,
    String,
  };

  class Base
  {
  public:
    DISALLOW_COPY_AND_ASSIGN(Base);

  #if defined(SNEK_ENABLE_PROPERTY_CACHE)
    using property_cache_type = std::unordered_map<
      std::u32string,
      std::shared_ptr<Base>
    >;
  #endif

    explicit Base() {}

    virtual Kind kind() const = 0;

    virtual bool Equals(const Base& that) const = 0;

    virtual std::u32string ToString() const = 0;

    virtual std::u32string ToSource() const = 0;

#if defined(SNEK_ENABLE_PROPERTY_CACHE)
  private:
    mutable property_cache_type m_property_cache;
    friend std::optional<std::shared_ptr<Base>> GetProperty(
      const Runtime&,
      const std::shared_ptr<Base>&,
      const std::u32string&
    );
#endif
  };

  using ptr = std::shared_ptr<Base>;

  inline Kind KindOf(const ptr& value)
  {
    return value ? value->kind() : Kind::Null;
  }

  inline bool IsBoolean(const ptr& value)
  {
    return KindOf(value) == Kind::Boolean;
  }

  inline bool IsFloat(const ptr& value)
  {
    return KindOf(value) == Kind::Float;
  }

  inline bool IsFunction(const ptr& value)
  {
    return KindOf(value) == Kind::Function;
  }

  inline bool IsInt(const ptr& value)
  {
    return KindOf(value) == Kind::Int;
  }

  inline bool IsList(const ptr& value)
  {
    return KindOf(value) == Kind::List;
  }

  inline bool IsNull(const ptr& value)
  {
    return !value;
  }

  inline bool IsNumber(const ptr& value)
  {
    const auto kind = KindOf(value);

    return kind == Kind::Float || kind == Kind::Int;
  }

  inline bool IsRecord(const ptr& value)
  {
    return KindOf(value) == Kind::Record;
  }

  inline bool IsString(const ptr& value)
  {
    return KindOf(value) == Kind::String;
  }

  ptr
  GetPrototypeOf(
    const Runtime& runtime,
    const ptr& value
  );

  std::optional<ptr>
  GetProperty(
    const Runtime& runtime,
    const ptr& value,
    const std::u32string& name
  );

  ptr
  CallMethod(
    Runtime& runtime,
    const ptr& value,
    const std::u32string& name,
    const std::vector<ptr>& arguments = {},
    const std::optional<Position>& position = std::nullopt,
    bool tail_call = false
  );

  inline bool Equals(const ptr& a, const ptr& b)
  {
    if (!a)
    {
      return !b;
    }
    else if (!b)
    {
      return false;
    }

    return a->Equals(*b);
  }

  bool ToBoolean(const ptr& value);

  inline std::u32string ToString(const ptr& value)
  {
    return value ? value->ToString() : U"";
  }

  std::u32string ToString(Kind kind);

  inline std::u32string ToSource(const ptr& value)
  {
    return value ? value->ToSource() : U"null";
  }

  class Number : public Base
  {
  public:
    using int_type = std::int64_t;
    using float_type = double;

    explicit Number()
      : Base() {}

    virtual int_type ToInt() const = 0;

    virtual float_type ToFloat() const = 0;

    inline std::u32string ToSource() const override
    {
      return ToString();
    }
  };

  class Boolean final : public Base
  {
  public:
    using value_type = bool;

    explicit Boolean(bool value)
      : m_value(value) {}

    inline Kind kind() const override
    {
      return Kind::Boolean;
    }

    inline value_type value() const
    {
      return m_value;
    }

    bool Equals(const Base& that) const override;

    inline std::u32string ToString() const override
    {
      return m_value ? U"true" : U"false";
    }

    inline std::u32string ToSource() const override
    {
      return ToString();
    }

  private:
    const value_type m_value;
  };

  class Float final : public Number
  {
  public:
    using value_type = float_type;

    explicit Float(value_type value)
      : m_value(value) {}

    inline Kind kind() const override
    {
      return Kind::Float;
    }

    inline value_type value() const
    {
      return m_value;
    }

    int_type ToInt() const override;

    inline float_type ToFloat() const override
    {
      return m_value;
    }

    bool Equals(const Base& that) const override;

    std::u32string ToString() const override;

  private:
    const value_type m_value;
  };

  class Function : public Base
  {
  public:
    using callback_type = std::function<ptr(
      Runtime&,
      const std::vector<ptr>&
    )>;

    explicit Function() {}

    static std::shared_ptr<Function>
    MakeNative(
      const std::vector<Parameter>& parameters,
      const type::ptr& return_type,
      const callback_type& callback
    );

    static std::shared_ptr<Function>
    MakeScripted(
      const std::vector<Parameter>& parameters,
      const type::ptr& return_type,
      const parser::statement::ptr& body,
      const std::shared_ptr<Scope>& enclosing_scope
    );

    static std::shared_ptr<Function>
    Bind(
      const ptr& this_value,
      const std::shared_ptr<Function>& function
    );

    static ptr
    Call(
      Runtime& runtime,
      const std::shared_ptr<value::Function>& function,
      const std::vector<ptr>& arguments,
      bool tail_call = false,
      const std::optional<Position>& position = std::nullopt
    );

    inline Kind kind() const override
    {
      return Kind::Function;
    }

    virtual const std::vector<Parameter>& parameters() const = 0;

    virtual const type::ptr& return_type() const = 0;

    bool Equals(const Base& that) const override;

    std::u32string ToString() const override;

    inline std::u32string ToSource() const override
    {
      return ToString();
    }

  protected:
    virtual ptr Call(
      Runtime& runtime,
      const std::vector<ptr>& arguments,
      const std::optional<Position>& position
    ) const = 0;
  };

  class Int final : public Number
  {
  public:
    using value_type = int_type;

    explicit Int(value_type value)
      : m_value(value) {}

    inline Kind kind() const override
    {
      return Kind::Int;
    }

    inline value_type value() const
    {
      return m_value;
    }

    inline int_type ToInt() const override
    {
      return m_value;
    }

    inline float_type ToFloat() const override
    {
      return static_cast<float_type>(m_value);
    }

    bool Equals(const Base& that) const override;

    std::u32string ToString() const override;

  private:
    const std::int64_t m_value;
  };

  class List : public Base
  {
  public:
    using value_type = ptr;
    using size_type = std::size_t;

    static std::shared_ptr<List>
    Make(const std::vector<ptr>& elements);

    explicit List() {}

    inline Kind kind() const override
    {
      return Kind::List;
    }

    virtual size_type GetSize() const = 0;

    virtual value_type At(size_type index) const = 0;

    bool Equals(const Base& that) const override;

    std::u32string ToString() const override;

    std::u32string ToSource() const override;

    virtual std::vector<ptr> ToVector() const;
  };

  class Record final : public Base
  {
  public:
    using key_type = std::u32string;
    using mapped_type = ptr;
    using container_type = std::unordered_map<key_type, mapped_type>;
    using value_type = container_type::value_type;
    using iterator = container_type::const_iterator;

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

    inline iterator begin() const
    {
      return std::begin(m_fields);
    }

    inline iterator end() const
    {
      return std::end(m_fields);
    }

    std::optional<ptr> GetOwnProperty(const key_type& name) const;

    bool Equals(const Base& that) const override;

    std::u32string ToString() const override;

    std::u32string ToSource() const override;

  private:
    const container_type m_fields;
  };

  class String : public Base
  {
  public:
    using value_type = char32_t;
    using size_type = std::size_t;

    static ptr
    Make(const std::u32string& text);

    inline Kind kind() const override
    {
      return Kind::String;
    }

    virtual size_type GetLength() const = 0;

    virtual value_type At(size_type index) const = 0;

    bool Equals(const Base& that) const override;

    std::u32string ToSource() const override;
  };
}
