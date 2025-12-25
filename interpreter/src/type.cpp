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
#include <algorithm>

#include "snek/interpreter/resolve.hpp"
#include "snek/parser/utils.hpp"

#include "./resolve/utils.hpp"

namespace snek::interpreter::type
{
  ptr
  MakeOptional(const ptr& type)
  {
    return std::make_shared<Union>(std::vector<ptr>{
      type,
      std::make_shared<Builtin>(BuiltinKind::Void)
    });
  }

  ptr
  Reify(const Runtime& runtime, const std::vector<ptr>& types)
  {
    const auto size = types.size();

    if (size > 1)
    {
      std::vector<ptr> result;

      result.reserve(size);
      for (std::size_t i = 0; i < size; ++i)
      {
        result.push_back(utils::TypeOrAny(runtime, types[i]));
      }

      // TODO: Get rid of duplicates with equality comparison.
      return std::make_shared<Union>(types);
    }
    else if (size > 0)
    {
      return types[0];
    }

    return runtime.void_type();
  }
}
