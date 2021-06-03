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
#pragma once

#include <snek/type/base.hpp>

namespace snek::type
{
  enum class PrimitiveKind
  {
    Bin,
    Bool,
    Float,
    Int,
    Num,
    Record,
    Str,
    Void,
  };

  class Primitive final : public Base
  {
  public:
    explicit Primitive(PrimitiveKind primitive_kind);

    inline Kind kind() const
    {
      return Kind::Primitive;
    }

    inline PrimitiveKind primitive_kind() const
    {
      return m_primitive_kind;
    }

    bool matches(const std::shared_ptr<value::Base>& value) const;

    bool matches(const Ptr& type) const;

    inline std::u32string to_string() const
    {
      return to_string(m_primitive_kind);
    }

    static std::u32string to_string(PrimitiveKind primitive_kind);

  private:
    const PrimitiveKind m_primitive_kind;
  };

  using PrimitivePtr = std::shared_ptr<Primitive>;
}
