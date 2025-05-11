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
      const auto union_ = utils::As<Union>(that);
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
    return utils::Join(types(), U" | ");
  }
}
