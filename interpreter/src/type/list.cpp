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
  List::Accepts(const Runtime& runtime, const value::ptr& value) const
  {
    if (value::IsList(value))
    {
      const auto list = static_cast<value::List*>(value.get());
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
      return m_element_type->Accepts(utils::As<List>(that)->m_element_type);
    }
    else if (that->kind() == Kind::Tuple)
    {
      const auto tuple = utils::As<Tuple>(that);
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
      return utils::As<Builtin>(that)->builtin_kind() == BuiltinKind::List;
    }

    return false;
  }
}
