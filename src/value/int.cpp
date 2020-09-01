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
#include <snek/interpreter.hpp>
#include <snek/value/float.hpp>
#include <snek/value/int.hpp>

namespace snek::value
{
  Int::Int(value_type value)
    : m_value(value) {}

  std::shared_ptr<type::Base>
  Int::type(const Interpreter& interpreter) const
  {
    return interpreter.int_type();
  }

  bool
  Int::equals(const Ptr& that) const
  {
    const auto kind = that->kind();

    if (kind == Kind::Int)
    {
      return m_value == std::static_pointer_cast<Int>(that)->m_value;
    }
    else if (kind == Kind::Float)
    {
      return static_cast<Float::value_type>(m_value)
        == std::static_pointer_cast<Float>(that)->value();
    }

    return false;
  }

  std::u32string
  Int::to_string() const
  {
    static const char digitmap[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    const bool negative = m_value < 0;
    auto mag = static_cast<std::uint64_t>(negative ? -m_value : m_value);
    std::u32string result;

    if (mag != 0)
    {
      result.reserve(negative ? 21 : 20);
      do
      {
        result.insert(result.begin(), digitmap[mag % 10]);
        mag /= 10;
      }
      while (mag);
    } else {
      result.insert(result.begin(), '0');
    }
    if (negative)
    {
      result.insert(result.begin(), '-');
    }

    return result;
  }
}
