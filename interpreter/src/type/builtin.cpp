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
#include "snek/interpreter/runtime.hpp"

#include "./utils.hpp"

namespace snek::interpreter::type
{
  bool
  Builtin::Accepts(const Runtime&, const value::ptr& value) const
  {
    switch (m_kind)
    {
      case BuiltinKind::Boolean:
        return value::IsBoolean(value);

      case BuiltinKind::Float:
        return value::IsNumber(value);

      case BuiltinKind::Function:
        return value::IsFunction(value);

      case BuiltinKind::Int:
        return value::IsInt(value);

      case BuiltinKind::List:
        return value::IsList(value);

      case BuiltinKind::Number:
        return value::IsNumber(value);

      case BuiltinKind::Record:
        return value::IsRecord(value);

      case BuiltinKind::String:
        return value::IsString(value);

      case BuiltinKind::Void:
        return value::IsNull(value);
    }

    return false;
  }

  bool
  Builtin::Accepts(const ptr& that) const
  {
    if (!that || this == that.get())
    {
      return true;
    }
    else if (that->kind() == Kind::Builtin)
    {
      const auto that_kind = utils::As<Builtin>(that)->m_kind;

      switch (m_kind)
      {
        case BuiltinKind::Float:
        case BuiltinKind::Number:
          return (
            that_kind == BuiltinKind::Float ||
            that_kind == BuiltinKind::Int
          );

        default:
          return m_kind == that_kind;
      }
    }
    switch (m_kind)
    {
      case BuiltinKind::Boolean:
        return that->kind() == Kind::Boolean;

      case BuiltinKind::Function:
        return that->kind() == Kind::Function;

      case BuiltinKind::List:
        return that->kind() == Kind::List || that->kind() == Kind::Tuple;

      case BuiltinKind::Record:
        return that->kind() == Kind::Record;

      case BuiltinKind::String:
        return that->kind() == Kind::String;

      default:
        return false;
    }
  }

  std::u32string
  Builtin::ToString() const
  {
    switch (m_kind)
    {
      case BuiltinKind::Boolean:
        return U"Boolean";

      case BuiltinKind::Float:
        return U"Float";

      case BuiltinKind::Function:
        return U"Function";

      case BuiltinKind::Int:
        return U"Int";

      case BuiltinKind::List:
        return U"List";

      case BuiltinKind::Number:
        return U"Number";

      case BuiltinKind::Record:
        return U"Record";

      case BuiltinKind::String:
        return U"String";

      case BuiltinKind::Void:
        return U"null";
    }

    return U"unknown";
  }
}
