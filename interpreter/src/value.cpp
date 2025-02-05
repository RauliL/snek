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
#include "snek/error.hpp"
#include "snek/interpreter/runtime.hpp"

namespace snek::interpreter::value
{
  template<class T>
  static inline const T*
  As(const ptr& value)
  {
    return static_cast<const T*>(value.get());
  }

  ptr
  GetPrototypeOf(const Runtime& runtime, const ptr& value)
  {
    const auto kind = KindOf(value);

    if (kind == Kind::Record)
    {
      if (const auto prototype = As<Record>(value)->GetOwnProperty(U"[[Prototype]]"))
      {
        return *prototype;
      }
    }

    switch (kind)
    {
      case Kind::Boolean:
        return runtime.boolean_prototype();

      case Kind::Float:
        return runtime.float_prototype();

      case Kind::Function:
        return runtime.function_prototype();

      case Kind::Int:
        return runtime.int_prototype();

      case Kind::List:
        return runtime.list_prototype();

      case Kind::Record:
        return runtime.record_prototype();

      case Kind::String:
        return runtime.string_prototype();

      default:
        return runtime.object_prototype();
    }
  }


  std::optional<ptr>
  GetProperty(
    const Runtime& runtime,
    const ptr& value,
    const std::u32string& name
  )
  {
    const auto kind = KindOf(value);

#if defined(SNEK_ENABLE_PROPERTY_CACHE)
    if (value)
    {
      const auto cached_property = value->m_property_cache.find(name);

      if (cached_property != std::end(value->m_property_cache))
      {
        return cached_property->second;
      }
    }
#endif
    if (kind == Kind::Record)
    {
      if (const auto property = As<Record>(value)->GetOwnProperty(name))
      {
        return *property;
      }
    }
    for (
      auto prototype = GetPrototypeOf(runtime, value);
      IsRecord(prototype);
      prototype = GetPrototypeOf(runtime, prototype)
    )
    {
      const auto property = As<Record>(prototype)->GetOwnProperty(name);

      if (property)
      {
        if (IsFunction(*property))
        {
          const auto function = Function::Bind(
            value,
            std::static_pointer_cast<Function>(*property)
          );

#if defined(SNEK_ENABLE_PROPERTY_CACHE)
          if (value)
          {
            value->m_property_cache[name] = function;
          }
#endif

          return function;
        }
#if defined(SNEK_ENABLE_PROPERTY_CACHE)
        if (value)
        {
          value->m_property_cache[name] = *property;
        }
#endif

        return *property;
      }
    }

    return std::nullopt;
  }

  ptr
  CallMethod(
    Runtime& runtime,
    const ptr& value,
    const std::u32string& name,
    const std::vector<ptr>& arguments,
    const std::optional<Position>& position
  )
  {
    const auto property = GetProperty(runtime, value, name);

    if (!property)
    {
      throw Error{
        position,
        ToString(KindOf(value)) + U" has no property `" +
        name +
        U"'."
      };
    }
    else if (IsFunction(*property))
    {
      return As<Function>(*property)->Call(position, runtime, arguments);
    }

    throw Error{
      position,
      ToString(KindOf(*property)) + U" is not callable."
    };
  }

  bool
  ToBoolean(const ptr& value)
  {
    if (!value)
    {
      return false;
    }
    else if (value->kind() == Kind::Boolean)
    {
      return As<Boolean>(value)->value();
    }

    return true;
  }

  std::u32string
  ToString(Kind kind)
  {
    switch (kind)
    {
      case Kind::Boolean:
        return U"Boolean";

      case Kind::Float:
        return U"Float";

      case Kind::Function:
        return U"Function";

      case Kind::Int:
        return U"Int";

      case Kind::List:
        return U"List";

      case Kind::Null:
        return U"null";

      case Kind::Record:
        return U"Record";

      case Kind::String:
        return U"String";
    }

    return U"unknown";
  }
}
