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
#include "snek/interpreter/error.hpp"
#include "snek/interpreter/resolve.hpp"

namespace snek::interpreter
{
  static type::ptr
  ResolveFunction(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Type::Function* type
  )
  {
    return std::make_shared<type::Function>(
      ResolveParameterList(runtime, scope, type->signature.parameters),
      ResolveType(runtime, scope, type->signature.return_type)
    );
  }

  static type::ptr
  ResolveMultiple(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Type::Multiple* type
  )
  {
    type::Multiple::container_type resolved_types;

    resolved_types.reserve(type->types.size());
    for (const auto& subtype : type->types)
    {
      resolved_types.push_back(ResolveType(runtime, scope, subtype));
    }

    switch (type->multiple_kind)
    {
      case parser::Type::Multiple::MultipleKind::Intersection:
        return std::make_shared<type::Intersection>(resolved_types);

      case parser::Type::Multiple::MultipleKind::Tuple:
        return std::make_shared<type::Tuple>(resolved_types);

      case parser::Type::Multiple::MultipleKind::Union:
        return std::make_shared<type::Union>(resolved_types);
    }

    return nullptr;
  }

  static type::ptr
  ResolveNamed(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Type::Named* type
  )
  {
    if (!type->name.compare(U"any"))
    {
      return runtime.any_type();
    }
    else if (scope)
    {
      type::ptr slot;

      if (scope->FindType(type->name, slot))
      {
        return slot;
      }
    }

    throw runtime.MakeError(U"Unknown type: `" + type->name + U"'.");
  }

  static type::ptr
  ResolveRecord(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Type::Record* type
  )
  {
    type::Record::container_type fields;

    for (const auto& field : type->fields)
    {
      fields[field.first] = ResolveType(runtime, scope, field.second);
    }

    return std::make_shared<type::Record>(fields);
  }

  type::ptr
  ResolveType(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::type_ptr& type
  )
  {
    if (!type)
    {
      return nullptr;
    }

    switch (type->kind)
    {
      case parser::TypeKind::Boolean:
        return std::make_shared<type::Boolean>(
          parser::As<parser::Type::Boolean>(type)->value
        );

      case parser::TypeKind::Function:
        return ResolveFunction(
          runtime,
          scope,
          parser::As<parser::Type::Function>(type)
        );

      case parser::TypeKind::List:
        return std::make_shared<type::List>(
          ResolveType(
            runtime,
            scope,
            parser::As<parser::Type::List>(type)->element_type
          )
        );

      case parser::TypeKind::Multiple:
        return ResolveMultiple(
          runtime,
          scope,
          parser::As<parser::Type::Multiple>(type)
        );

      case parser::TypeKind::Named:
        return ResolveNamed(
          runtime,
          scope,
          parser::As<parser::Type::Named>(type)
        );

      case parser::TypeKind::Null:
        return runtime.void_type();

      case parser::TypeKind::Record:
        return ResolveRecord(
          runtime,
          scope,
          parser::As<parser::Type::Record>(type)
        );

      case parser::TypeKind::String:
        return std::make_shared<type::String>(
          parser::As<parser::Type::String>(type)->value
        );
    }

    return nullptr;
  }
}
