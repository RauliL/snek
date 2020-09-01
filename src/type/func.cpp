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
#include <snek/type/func.hpp>
#include <snek/value/func.hpp>

namespace snek::type
{
  Func::Func(
    const std::vector<Parameter>& parameters,
    const Ptr& return_type
  )
    : m_parameters(parameters)
    , m_return_type(return_type) {}

  bool
  Func::matches(const value::Ptr& value) const
  {
    value::FuncPtr value_func;

    if (value->kind() != value::Kind::Func)
    {
      return false;
    }
    value_func = std::static_pointer_cast<value::Func>(value);
    if (m_parameters.size() != value_func->parameters().size())
    {
      return false;
    }
    for (std::size_t i = 0; i < m_parameters.size(); ++i)
    {
      if (!m_parameters[i].type()->matches(value_func->parameters()[i].type()))
      {
        return false;
      }
    }
    if (const auto return_type = value_func->return_type())
    {
      return m_return_type->matches(*return_type);
    }

    return m_return_type->kind() == Kind::Any;
  }

  bool
  Func::matches(const Ptr& type) const
  {
    FuncPtr type_func;

    if (type->kind() != Kind::Func)
    {
      return false;
    }
    type_func = std::static_pointer_cast<Func>(type);
    if (m_parameters.size() != type_func->parameters().size())
    {
      return false;
    }
    for (std::size_t i = 0; i < m_parameters.size(); ++i)
    {
      if (!m_parameters[i].type()->matches(type_func->parameters()[i].type()))
      {
        return false;
      }
    }

    return m_return_type->matches(type_func->return_type());
  }

  std::u32string
  Func::to_string() const
  {
    bool first = true;
    std::u32string result;

    result += U"(";
    for (const auto& parameter : m_parameters)
    {
      if (first)
      {
        first = false;
      } else {
        result += U", ";
      }
      result += parameter.to_string();
    }
    result += U") -> ";
    result += m_return_type->to_string();

    return result;
  }
}
