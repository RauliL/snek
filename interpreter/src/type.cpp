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
#include "snek/parser/utils.hpp"

namespace snek::interpreter::type
{
  template<class T>
  inline const T*
  As(const ptr& type)
  {
    return static_cast<const T*>(type.get());
  }

  template<class T>
  inline const T*
  As(const value::ptr& type)
  {
    return static_cast<const T*>(type.get());
  }

  static inline std::u32string
  Join(const Multiple::container_type& types, const char32_t* separator)
  {
    std::u32string result;
    bool first = true;

    for (const auto& type : types)
    {
      if (first)
      {
        first = false;
      } else {
        result.append(separator);
      }
      result.append(type->ToString());
    }

    return result;
  }

  ptr
  Reify(const Runtime& runtime, const std::vector<ptr>& types)
  {
    const auto size = types.size();

    if (size == 0)
    {
      return runtime.void_type();
    }
    else if (size == 1)
    {
      return types[0];
    } else {
      std::vector<ptr> result;

      result.reserve(types.size());
      for (std::size_t i = 0; i < size; ++i)
      {
        const auto& type = types[i];

        result.push_back(type ? type : runtime.any_type());
      }

      // TODO: Get rid of duplicates with equality comparison.
      return std::make_shared<Union>(types);
    }
  }

  bool
  Boolean::Accepts(const Runtime&, const value::ptr& value) const
  {
    if (value::IsBoolean(value))
    {
      return As<value::Boolean>(value)->value() == m_value;
    }

    return false;
  }

  bool
  Boolean::Accepts(const ptr& that) const
  {
    if (!that || this == that.get())
    {
      return true;
    }
    else if (that->kind() == Kind::Boolean)
    {
      return m_value == As<Boolean>(that)->m_value;
    }
    else if (that->kind() == Kind::Builtin)
    {
      return As<Builtin>(that)->builtin_kind() == BuiltinKind::Boolean;
    }

    return false;
  }

  bool
  Builtin::Accepts(const Runtime&, const value::ptr& value) const
  {
    switch (m_kind)
    {
      case BuiltinKind::Boolean:
        return value::IsBoolean(value);

      case BuiltinKind::Float:
        return value::IsFloat(value);

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
      const auto that_kind = As<Builtin>(that)->m_kind;

      return (
        m_kind == that_kind ||
        (
          m_kind == BuiltinKind::Number &&
          (that_kind == BuiltinKind::Float || that_kind == BuiltinKind::Int)
        )
      );
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

  bool
  Function::Accepts(const Runtime&, const value::ptr& value) const
  {
    if (value::IsFunction(value))
    {
      const auto size = m_parameters.size();
      const auto function = As<value::Function>(value);
      const auto& parameters = function->parameters();
      const auto& return_type = function->return_type();

      if (!m_return_type->Accepts(return_type))
      {
        return false;
      }
      if (size != parameters.size())
      {
        return false;
      }
      for (std::size_t i = 0; i < size; ++i)
      {
        if (!m_parameters[i].Accepts(parameters[i]))
        {
          return false;
        }
      }

      return true;
    }

    return false;
  }

  bool
  Function::Accepts(const ptr& that) const
  {
    if (!that || this == that.get())
    {
      return true;
    }
    else if (that->kind() == Kind::Function)
    {
      const auto function = As<Function>(that);
      const auto size = m_parameters.size();

      if (size != function->m_parameters.size())
      {
        return false;
      }
      for (std::size_t i = 0; i < size; ++i)
      {
        if (!m_parameters[i].Accepts(function->m_parameters[i]))
        {
          return false;
        }
      }
      if (m_return_type && !m_return_type->Accepts(function->m_return_type))
      {
        return false;
      }

      return true;
    }
    else if (that->kind() == Kind::Builtin)
    {
      return As<Builtin>(that)->builtin_kind() == BuiltinKind::Function;
    }

    return false;
  }

  std::u32string
  Function::ToString() const
  {
    std::u32string result(1, U'(');

    for (std::size_t i = 0; i < m_parameters.size(); ++i)
    {
      if (i > 0)
      {
        result.append(U", ");
      }
      result.append(m_parameters[i].ToString());
    }
    result.append(U") => ");
    result.append(m_return_type ? m_return_type->ToString() : U"null");

    return result;
  }

  bool
  Intersection::Accepts(const Runtime& runtime, const value::ptr& value) const
  {
    for (const auto& type : types())
    {
      if (!type->Accepts(runtime, value))
      {
        return false;
      }
    }

    return true;
  }

  bool
  Intersection::Accepts(const ptr& that) const
  {
    if (!that || this == that.get())
    {
      return true;
    }
    else if (that->kind() == Kind::Intersection)
    {
      const auto intersection = As<Intersection>(that);
      const auto& this_types = types();
      const auto& that_types = intersection->types();
      const auto this_size = this_types.size();
      const auto that_size = that_types.size();

      for (std::size_t i = 0; i < this_size; ++i)
      {
        bool found = false;

        for (std::size_t j = 0; j < that_size; ++j)
        {
          if (this_types[i]->Accepts(that_types[j]))
          {
            found = true;
            break;
          }
        }
        if (!found)
        {
          return false;
        }
      }

      return true;
    } else {
      for (const auto& type : types())
      {
        if (!type->Accepts(that))
        {
          return false;
        }
      }

      return true;
    }
  }

  std::u32string
  Intersection::ToString() const
  {
    return Join(types(), U" & ");
  }

  bool
  List::Accepts(const Runtime& runtime, const value::ptr& value) const
  {
    if (value::IsList(value))
    {
      const auto list = As<value::List>(value);
      const auto size = list->GetSize();

      for (std::size_t i = 0; i < size; ++i)
      {
        if (!m_element_type->Accepts(runtime, list->At(i)))
        {
          return false;
        }
      }

      return true;
    }

    return false;
  }

  bool
  List::Accepts(const ptr& that) const
  {
    if (!that || this == that.get())
    {
      return true;
    }
    else if (that->kind() == Kind::List)
    {
      return m_element_type->Accepts(As<List>(that)->m_element_type);
    }
    else if (that->kind() == Kind::Tuple)
    {
      const auto tuple = As<Tuple>(that);
      const auto& types = tuple->types();
      const auto size = types.size();

      for (std::size_t i = 0; i < size; ++i)
      {
        if (!m_element_type->Accepts(types[i]))
        {
          return false;
        }
      }

      return true;
    }
    else if (that->kind() == Kind::Builtin)
    {
      return As<Builtin>(that)->builtin_kind() == BuiltinKind::List;
    }

    return false;
  }

  bool
  Record::Accepts(const Runtime& runtime, const value::ptr& value) const
  {
    if (!value::IsRecord(value))
    {
      return false;
    }
    for (const auto& field : m_fields)
    {
      if (const auto property = value::GetProperty(runtime, value, field.first))
      {
        if (!field.second->Accepts(runtime, *property))
        {
          return false;
        }
      } else {
        return false;
      }
    }

    return true;
  }

  bool
  Record::Accepts(const ptr& that) const
  {
    if (!that || this == that.get())
    {
      return true;
    }
    else if (that->kind() == Kind::Record)
    {
      const auto record = As<Record>(that);

      for (const auto& field : m_fields)
      {
        const auto it = record->m_fields.find(field.first);

        if (
          it == std::end(record->m_fields) ||
          !field.second->Accepts(it->second)
        )
        {
          return false;
        }

        return true;
      }
    }
    else if (that->kind() == Kind::Builtin)
    {
      return As<Builtin>(that)->builtin_kind() == BuiltinKind::Record;
    }

    return false;
  }

  std::u32string
  Record::ToString() const
  {
    bool first = true;
    std::u32string result(1, U'{');

    for (const auto& field : m_fields)
    {
      if (first)
      {
        first = false;
      } else {
        result.append(U", ");
      }
      if (parser::utils::IsId(field.first))
      {
        result.append(field.first);
      } else {
        result.append(parser::utils::ToJsonString(field.first));
      }
      result.append(U": ");
      result.append(field.second->ToString());
    }
    result.append(1, U'}');

    return result;
  }

  bool
  String::Accepts(const Runtime&, const value::ptr& value) const
  {
    if (value::IsString(value))
    {
      return !m_value.compare(
        static_cast<const value::String*>(value.get())->value()
      );
    }

    return false;
  }

  bool
  String::Accepts(const ptr& that) const
  {
    if (!that || this == that.get())
    {
      return true;
    }
    else if (that->kind() == Kind::String)
    {
      return !m_value.compare(As<String>(that)->m_value);
    }
    else if (that->kind() == Kind::Builtin)
    {
      return As<Builtin>(that)->builtin_kind() == BuiltinKind::String;
    }

    return false;
  }

  std::u32string
  String::ToString() const
  {
    return parser::utils::ToJsonString(m_value);
  }

  bool
  Tuple::Accepts(const Runtime& runtime, const value::ptr& value) const
  {
    if (value::IsList(value))
    {
      const auto& subtypes = types();
      const auto list = static_cast<const value::List*>(value.get());
      const auto size = list->GetSize();

      if (subtypes.size() != size)
      {
        return false;
      }
      for (std::size_t i = 0; i < size; ++i)
      {
        if (!subtypes[i]->Accepts(runtime, list->At(i)))
        {
          return false;
        }
      }

      return true;
    }

    return false;
  }

  bool
  Tuple::Accepts(const ptr& that) const
  {
    if (!that || this == that.get())
    {
      return true;
    }
    else if (that->kind() == Kind::Tuple)
    {
      const auto tuple = As<Tuple>(that);
      const auto& this_types = types();
      const auto& that_types = tuple->types();
      const auto size = this_types.size();

      if (size != that_types.size())
      {
        return false;
      }
      for (std::size_t i = 0; i < size; ++i)
      {
        if (!this_types[i]->Accepts(that_types[i]))
        {
          return false;
        }
      }

      return true;
    }
    else if (that->kind() == Kind::List)
    {
      const auto& element_type = As<List>(that)->element_type();
      const auto& subtypes = types();
      const auto size = subtypes.size();

      for (std::size_t i = 0; i < size; ++i)
      {
        if (!subtypes[i]->Accepts(element_type))
        {
          return false;
        }
      }

      return true;
    }
    else if (that->kind() == Kind::Builtin)
    {
      return As<Builtin>(that)->builtin_kind() == BuiltinKind::List;
    }

    return false;
  }

  std::u32string
  Tuple::ToString() const
  {
    return U"[" + Join(types(), U", ") + U"]";
  }

  bool
  Union::Accepts(const Runtime& runtime, const value::ptr& value) const
  {
    for (const auto& type : types())
    {
      if (type->Accepts(runtime, value))
      {
        return true;
      }
    }

    return false;
  }

  bool
  Union::Accepts(const ptr& that) const
  {
    if (!that || this == that.get())
    {
      return true;
    }
    else if (that->kind() == Kind::Union)
    {
      const auto union_ = As<Union>(that);
      const auto& this_types = types();
      const auto& that_types = union_->types();
      const auto this_size = this_types.size();
      const auto that_size = that_types.size();

      for (std::size_t i = 0; i < this_size; ++i)
      {
        for (std::size_t j = 0; j < that_size; ++j)
        {
          if (this_types[i]->Accepts(that_types[j]))
          {
            return true;
          }
        }
      }

      return false;
    } else {
      for (const auto& type : types())
      {
        if (type->Accepts(that))
        {
          return true;
        }
      }

      return false;
    }
  }

  std::u32string
  Union::ToString() const
  {
    return Join(types(), U" | ");
  }
}
