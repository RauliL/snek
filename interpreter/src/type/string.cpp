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

#include "./utils.hpp"

namespace snek::interpreter::type
{
  bool
  String::Accepts(const Runtime&, const value::ptr& value) const
  {
    if (value::IsString(value))
    {
      return !m_value.compare(
        static_cast<const value::String*>(value.get())->ToString()
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
      return !m_value.compare(utils::As<String>(that)->m_value);
    }
    else if (that->kind() == Kind::Builtin)
    {
      return utils::As<Builtin>(that)->builtin_kind() == BuiltinKind::String;
    }

    return false;
  }

  std::u32string
  String::ToString() const
  {
    return parser::utils::ToJsonString(m_value);
  }
}
