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
#include "snek/parser/field.hpp"

namespace snek::interpreter
{
  template<class T>
  static inline const T*
  As(const type::ptr& type)
  {
    return static_cast<const T*>(type.get());
  }

  static bool
  ResolveComputed(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Field::Computed* field,
    type::Record::container_type& resolved_fields
  )
  {
    const auto key_type = ResolveExpression(runtime, scope, field->key);

    if (key_type && key_type->kind() == type::Kind::String)
    {
      const auto value_type = ResolveExpression(
        runtime,
        scope,
        field->value
      );

      if (!value_type)
      {
        return false;
      }
      resolved_fields[As<type::String>(key_type)->value()] = value_type;

      return true;
    }

    return false;
  }

  static bool
  ResolveFunction(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Field::Function* field,
    type::Record::container_type& resolved_fields
  )
  {
    resolved_fields[field->name] = std::make_shared<type::Function>(
      ResolveParameterList(runtime, scope, field->signature.parameters),
      ResolveType(runtime, scope, field->signature.return_type)
    );

    return true;
  }

  static bool
  ResolveNamed(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Field::Named* field,
    type::Record::container_type& resolved_fields
  )
  {
    const auto type = ResolveExpression(runtime, scope, field->value);

    if (!type)
    {
      return false;
    }
    resolved_fields[field->name] = type;

    return true;
  }

  static bool
  ResolveShorthand(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Field::Shorthand* field,
    type::Record::container_type& resolved_fields
  )
  {
    const auto type = ResolveExpression(
      runtime,
      scope,
      std::make_shared<parser::Expression::Id>(
        field->position,
        field->name
      )
    );

    if (type && type->kind() == type::Kind::String)
    {
      const auto value = As<type::String>(type)->value();

      resolved_fields[value] = type;

      return true;
    }

    return false;
  }

  static bool
  ResolveSpread(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Field::Spread* field,
    type::Record::container_type& resolved_fields
  )
  {
    const auto type = ResolveExpression(runtime, scope, field->expression);

    if (type && type->kind() == type::Kind::Record)
    {
      for (const auto& resolved_field : As<type::Record>(type)->fields())
      {
        resolved_fields[resolved_field.first] = resolved_field.second;
      }

      return true;
    }

    return false;
  }

  bool
  ResolveField(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::field_ptr& field,
    type::Record::container_type& resolved_fields
  )
  {
    switch (field->kind)
    {
      case parser::FieldKind::Computed:
        return ResolveComputed(
          runtime,
          scope,
          As<parser::Field::Computed>(field),
          resolved_fields
        );

      case parser::FieldKind::Function:
        return ResolveFunction(
          runtime,
          scope,
          As<parser::Field::Function>(field),
          resolved_fields
        );
        break;

      case parser::FieldKind::Named:
        return ResolveNamed(
          runtime,
          scope,
          As<parser::Field::Named>(field),
          resolved_fields
        );

      case parser::FieldKind::Shorthand:
        return ResolveShorthand(
          runtime,
          scope,
          As<parser::Field::Shorthand>(field),
          resolved_fields
        );

      case parser::FieldKind::Spread:
        return ResolveSpread(
          runtime,
          scope,
          As<parser::Field::Spread>(field),
          resolved_fields
        );
    }

    return false;
  }
}
