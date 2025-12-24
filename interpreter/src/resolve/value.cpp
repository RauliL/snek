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
#include "snek/interpreter/resolve.hpp"

#include "./utils.hpp"

namespace snek::interpreter
{
  using namespace snek::interpreter::value;

  template<class T>
  static inline const T*
  As(const ptr& value)
  {
    return static_cast<const T*>(value.get());
  }

  static inline type::ptr
  ResolveFunction(const Runtime& runtime, const Function* function)
  {
    return std::make_shared<type::Function>(
      function->parameters(),
      utils::TypeOrAny(runtime, function->return_type())
    );
  }

  static type::ptr
  ResolveList(const Runtime& runtime, const List* list)
  {
    const auto size = list->GetSize();

    if (size > 0)
    {
      std::vector<type::ptr> types;

      types.reserve(size);
      for (std::size_t i = 0; i < size; ++i)
      {
        types.push_back(
          utils::TypeOrAny(runtime, ResolveValue(runtime, list->At(i)))
        );
      }

      return std::make_shared<type::Tuple>(types);
    }

    return runtime.list_type();
  }

  static type::ptr
  ResolveRecord(const Runtime& runtime, const Record* record)
  {
    type::Record::container_type fields;

    for (const auto& field : record->GetOwnPropertyNames())
    {
      // TODO: Special handling for "[[Prototype]]".
      fields[field] = utils::TypeOrAny(
        runtime,
        ResolveValue(runtime, *record->GetOwnProperty(field))
      );
    }

    return std::make_shared<type::Record>(fields);
  }

  static inline type::ptr
  ResolveString(const Runtime& runtime, const String* string)
  {
    return std::make_shared<type::String>(string->ToString());
  }

  type::ptr
  ResolveValue(const Runtime& runtime, const ptr& value)
  {
    type::ptr type;

#if defined(SNEK_ENABLE_TYPE_CACHE)
    if (value->m_cached_type)
    {
      return value->m_cached_type;
    }
#endif

    switch (KindOf(value))
    {
      case Kind::Boolean:
        type = runtime.boolean_type();
        break;

      case Kind::Float:
        type = runtime.float_type();
        break;

      case Kind::Function:
        type = ResolveFunction(runtime, As<Function>(value));
        break;

      case Kind::Int:
        type = runtime.int_type();
        break;

      case Kind::List:
        type = ResolveList(runtime, As<List>(value));
        break;

      case Kind::Null:
        type = runtime.void_type();
        break;

      case Kind::Record:
        type = ResolveRecord(runtime, As<Record>(value));
        break;

      case Kind::String:
        type = ResolveString(runtime, As<String>(value));
        break;
    }

#if defined(SNEK_ENABLE_TYPE_CACHE)
    value->m_cached_type = type;
#endif

    return type;
  }
}
