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
  static bool
  TestFunctions(
    const std::vector<Parameter>& a,
    const std::vector<Parameter>& b,
    const ptr& a_return_type,
    const ptr& b_return_type
  )
  {
    const auto a_size = a.size();
    const auto b_size = b.size();
    const auto size = std::min(a_size, b_size);

    // TODO: Add special handling for rest parameters.
    if (a_size < b_size)
    {
      return false;
    }

    if (a_return_type && !a_return_type->Accepts(b_return_type))
    {
      return false;
    }

    for (std::size_t i = 0; i < size; ++i)
    {
      if (!a[i].Accepts(b[i]))
      {
        return false;
      }
    }

    return true;
  }

  bool
  Function::Accepts(const Runtime&, const value::ptr& value) const
  {
    if (value::IsFunction(value))
    {
      const auto function = static_cast<value::Function*>(value.get());

      return TestFunctions(
        m_parameters,
        function->parameters(),
        m_return_type,
        function->return_type()
      );
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
      const auto function = utils::As<Function>(that);

      return TestFunctions(
        m_parameters,
        function->m_parameters,
        m_return_type,
        function->m_return_type
      );
    }
    else if (that->kind() == Kind::Builtin)
    {
      return utils::As<Builtin>(that)->builtin_kind() == BuiltinKind::Function;
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

    return result
      .append(U") => ")
      .append(m_return_type ? m_return_type->ToString() : U"any");
  }
}
