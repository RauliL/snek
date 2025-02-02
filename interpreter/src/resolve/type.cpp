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
#include "snek/interpreter/resolve.hpp"

namespace snek::interpreter
{
  using namespace parser::type;

  template <class T>
  static inline const T*
  As(const ptr& type)
  {
    return static_cast<const T*>(type.get());
  }

  static type::ptr
  ResolveFunction(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const Function* type
  )
  {
    return std::make_shared<type::Function>(
      ResolveParameterList(runtime, scope, type->parameters()),
      ResolveType(runtime, scope, type->return_type())
    );
  }

  static type::ptr
  ResolveMultiple(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const Multiple* type
  )
  {
    const auto& types = type->types();
    type::Multiple::container_type resolved_types;

    resolved_types.reserve(types.size());
    for (const auto& subtype : type->types())
    {
      resolved_types.push_back(ResolveType(runtime, scope, subtype));
    }

    switch (type->multiple_kind())
    {
      case Multiple::MultipleKind::Intersection:
        return std::make_shared<type::Intersection>(resolved_types);

      case Multiple::MultipleKind::Tuple:
        return std::make_shared<type::Tuple>(resolved_types);

      case Multiple::MultipleKind::Union:
        return std::make_shared<type::Union>(resolved_types);
    }

    return nullptr;
  }

  static type::ptr
  ResolveNamed(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const Named* type
  )
  {
    const auto& name = type->name();

    if (!name.compare(U"any"))
    {
      return runtime.any_type();
    }
    else if (scope)
    {
      type::ptr slot;

      if (scope->FindType(name, slot))
      {
        return slot;
      }
    }

    throw Error{ type->position(), U"Unknown type: `" + name + U"'." };
  }

  static type::ptr
  ResolveRecord(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const Record* type
  )
  {
    type::Record::container_type fields;

    for (const auto& field : type->fields())
    {
      fields[field.first] = ResolveType(runtime, scope, field.second);
    }

    return std::make_shared<type::Record>(fields);
  }

  type::ptr
  ResolveType(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::type::ptr& type
  )
  {
    if (!type)
    {
      return nullptr;
    }

    switch (type->kind())
    {
      case Kind::Boolean:
        return std::make_shared<type::Boolean>(As<Boolean>(type)->value());

      case Kind::Function:
        return ResolveFunction(runtime, scope, As<Function>(type));

      case Kind::List:
        return std::make_shared<type::List>(
          ResolveType(
            runtime,
            scope,
            As<List>(type)->element_type()
          )
        );

      case Kind::Multiple:
        return ResolveMultiple(runtime, scope, As<Multiple>(type));

      case Kind::Named:
        return ResolveNamed(runtime, scope, As<Named>(type));

      case Kind::Null:
        return runtime.void_type();

      case Kind::Record:
        return ResolveRecord(runtime, scope, As<Record>(type));

      case Kind::String:
        return std::make_shared<type::String>(As<String>(type)->value());
    }

    return nullptr;
  }
}
