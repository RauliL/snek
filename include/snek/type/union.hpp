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

#include <vector>

#include <snek/type/base.hpp>

namespace snek::type
{
  class Union final : public Base
  {
  public:
    using value_type = Ptr;
    using container_type = std::vector<value_type>;
    using iterator = container_type::const_iterator;
    using const_iterator = container_type::const_iterator;
    using reverse_iterator = container_type::const_reverse_iterator;
    using const_reverse_iterator = container_type::const_reverse_iterator;

    explicit Union(const container_type& types);

    inline Kind kind() const
    {
      return Kind::Union;
    }

    inline const container_type& types() const
    {
      return m_types;
    }

    bool matches(const std::shared_ptr<value::Base>& value) const;

    bool matches(const Ptr& value) const;

    std::u32string to_string() const;

    inline const_iterator begin() const
    {
      return std::begin(m_types);
    }

    inline const_iterator end() const
    {
      return std::end(m_types);
    }

    inline const_reverse_iterator rbegin() const
    {
      return std::rbegin(m_types);
    }

    inline const_reverse_iterator rend() const
    {
      return std::rend(m_types);
    }

  private:
    const container_type m_types;
  };

  using UnionPtr = std::shared_ptr<Union>;
}
