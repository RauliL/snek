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
#include <snek/type/tuple.hpp>
#include <snek/value/list.hpp>

namespace snek::type
{
  Tuple::Tuple(const container_type& types)
    : m_types(types) {}

  bool
  Tuple::matches(const value::Ptr& value) const
  {
    value::ListPtr value_list;

    if (value->kind() != value::Kind::List)
    {
      return false;
    }
    value_list = std::static_pointer_cast<value::List>(value);
    if (value_list->elements().size() != m_types.size())
    {
      return false;
    }
    for (std::size_t i = 0; i < m_types.size(); ++i)
    {
      if (!m_types[i]->matches(value_list->elements()[i]))
      {
        return false;
      }
    }

    return true;
  }

  bool
  Tuple::matches(const Ptr& type) const
  {
    TuplePtr that;

    if (type->kind() != Kind::Tuple)
    {
      return false;
    }
    that = std::static_pointer_cast<Tuple>(type);
    if (m_types.size() != that->m_types.size())
    {
      return false;
    }
    for (std::size_t i = 0; i < m_types.size(); ++i)
    {
      if (!m_types[i]->matches(that->m_types[i]))
      {
        return false;
      }
    }

    return true;
  }

  std::u32string
  Tuple::to_string() const
  {
    bool first = true;
    std::u32string result;

    result += U"[";
    for (const auto& type : m_types)
    {
      if (first)
      {
        first = false;
      } else {
        result += U", ";
      }
      result += type->to_string();
    }
    result += U"]";

    return result;
  }
}
