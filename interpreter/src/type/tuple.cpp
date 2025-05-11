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
      const auto tuple = utils::As<Tuple>(that);
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
      const auto& element_type = utils::As<List>(that)->element_type();
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
      return utils::As<Builtin>(that)->builtin_kind() == BuiltinKind::List;
    }

    return false;
  }

  std::u32string
  Tuple::ToString() const
  {
    return U"[" + utils::Join(types(), U", ") + U"]";
  }
}
