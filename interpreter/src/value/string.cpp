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
#include "snek/interpreter/value.hpp"
#include "snek/parser/utils.hpp"

namespace snek::interpreter::value
{
  namespace
  {
    class StringWrapper final : public String
    {
    public:
      explicit StringWrapper(const std::u32string& text)
        : m_text(text) {}

      inline size_type GetLength() const override
      {
        return m_text.length();
      }

      inline value_type At(size_type index) const override
      {
        return m_text[index];
      }

      inline std::u32string ToString() const override
      {
        return m_text;
      }

    private:
      const std::u32string m_text;
    };
  }

  ptr
  String::Make(const std::u32string& text)
  {
    return std::make_shared<StringWrapper>(text);
  }

  bool
  String::Equals(const Base& that) const
  {
    if (this == &that)
    {
      return true;
    }
    else if (that.kind() == Kind::String)
    {
      const auto s = static_cast<const String*>(&that);
      const auto length = GetLength();

      if (length != s->GetLength())
      {
        return false;
      }
      for (size_type i = 0; i < length; ++i)
      {
        if (At(i) != s->At(i))
        {
          return false;
        }
      }

      return true;
    }

    return false;
  }

  std::u32string
  String::ToSource() const
  {
    return parser::utils::ToJsonString(ToString());
  }
}
