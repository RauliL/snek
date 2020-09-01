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
#include <cmath>

#include <snek/interpreter.hpp>
#include <snek/value/float.hpp>
#include <snek/value/int.hpp>

namespace snek::value
{
  Float::Float(value_type value)
    : m_value(value) {}

  std::shared_ptr<type::Base>
  Float::type(const Interpreter& interpreter) const
  {
    return interpreter.float_type();
  }

  bool
  Float::equals(const Ptr& that) const
  {
    const auto kind = that->kind();

    if (kind == Kind::Float)
    {
      return m_value == std::static_pointer_cast<Float>(that)->m_value;
    }
    else if (kind == Kind::Int)
    {
      return m_value == static_cast<value_type>(
        std::static_pointer_cast<Int>(that)->value()
      );
    }

    return false;
  }

  std::u32string
  Float::to_string() const
  {
    static const std::u32string string_nan = U"NaN";
    static const std::u32string string_inf = U"Inf";
    static const std::u32string string_inf_neg = U"-Inf";
    char buffer[20];

    if (std::isnan(m_value))
    {
      return string_nan;
    }
    else if (std::isinf(m_value))
    {
      return m_value < 0.0 ? string_inf_neg : string_inf;
    }
    std::snprintf(buffer, sizeof(buffer), "%g", m_value);

    return peelo::unicode::encoding::utf8::decode(buffer);
  }
}
