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

namespace snek::interpreter::value
{
  namespace
  {
    class VectorList final : public List
    {
    public:
      explicit VectorList(const container_type& elements)
        : m_elements(elements) {}

      inline size_type
      GetSize() const override
      {
        return m_elements.size();
      }

      inline value_type
      At(size_type index) const override
      {
        return m_elements[index];
      }

      inline container_type
      ToVector() const override
      {
        return m_elements;
      }

    private:
      const container_type m_elements;
    };
  }

  list_ptr
  List::Make(const container_type& elements)
  {
    return std::make_shared<VectorList>(elements);
  }

  void
  List::ForEach(
    const std::function<void(const value_type&, size_type)>& callback
  ) const
  {
    const auto size = GetSize();

    for (size_type i = 0; i < size; ++i)
    {
      callback(At(i), i);
    }
  }

  bool
  List::Equals(const Base& that) const
  {
    if (that.kind() == Kind::List)
    {
      const auto that_list = static_cast<const List*>(&that);
      const auto size = GetSize();

      if (this == that_list)
      {
        return true;
      }
      else if (size != that_list->GetSize())
      {
        return false;
      }
      for (size_type i = 0; i < size; ++i)
      {
        if (!value::Equals(At(i), that_list->At(i)))
        {
          return false;
        }
      }

      return true;
    }

    return false;
  }

  std::u32string
  List::ToString() const
  {
    std::u32string result;
    const auto size = GetSize();

    for (size_type i = 0; i < size; ++i)
    {
      if (i > 0)
      {
        result.append(U", ");
      }
      result.append(value::ToString(At(i)));
    }

    return result;
  }

  std::u32string
  List::ToSource() const
  {
    std::u32string result(1, U'[');
    const auto size = GetSize();

    for (size_type i = 0; i < size; ++i)
    {
      if (i > 0)
      {
        result.append(U", ");
      }
      result.append(value::ToSource(At(i)));
    }

    return result.append(1, U']');
  }

  List::container_type
  List::ToVector() const
  {
    const auto size = GetSize();
    std::vector<ptr> result;

    result.reserve(size);
    for (std::size_t i = 0; i < size; ++i)
    {
      result.push_back(At(i));
    }

    return result;
  }
}
