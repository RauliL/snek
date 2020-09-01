/*
 * Copyright (c) 2020, Rauli Laine
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
#include <snek/type/primitive.hpp>
#include <snek/value/base.hpp>

namespace snek::type
{
  Primitive::Primitive(PrimitiveKind primitive_kind)
    : m_primitive_kind(primitive_kind) {}

  bool
  Primitive::matches(const value::Ptr& value) const
  {
    const auto kind = value->kind();

    switch (m_primitive_kind)
    {
      case PrimitiveKind::Bool:
        return kind == value::Kind::Bool;

      case PrimitiveKind::Float:
        return kind == value::Kind::Float;

      case PrimitiveKind::Int:
        return kind == value::Kind::Int;

      case PrimitiveKind::Num:
        return kind == value::Kind::Float || kind == value::Kind::Int;

      case PrimitiveKind::Str:
        return kind == value::Kind::Str;

      case PrimitiveKind::Void:
        return kind == value::Kind::Null;
    }

    return false;
  }

  bool
  Primitive::matches(const Ptr& type) const
  {
    return type->kind() == Kind::Primitive &&
      std::static_pointer_cast<Primitive>(type)->primitive_kind() ==
        m_primitive_kind;
  }

  std::u32string
  Primitive::to_string(PrimitiveKind primitive_kind)
  {
    switch (primitive_kind)
    {
      case PrimitiveKind::Bool:
        return U"Bool";

      case PrimitiveKind::Float:
        return U"Float";

      case PrimitiveKind::Int:
        return U"Int";

      case PrimitiveKind::Num:
        return U"Num";

      case PrimitiveKind::Str:
        return U"Str";

      case PrimitiveKind::Void:
        return U"Void";
    }

    return U"Unknown";
  }
}
