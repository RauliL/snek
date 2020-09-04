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

#include <snek/value/base.hpp>

namespace snek
{
  class Message
  {
  public:
    using value_type = std::shared_ptr<value::Base>;
    using reference = value_type&;
    using const_reference = const value_type&;
    using container_type = std::vector<value_type>;
    using size_type = container_type::size_type;
    using iterator = container_type::iterator;
    using const_iterator = container_type::const_iterator;
    using reverse_iterator = container_type::reverse_iterator;
    using const_reverse_iterator = container_type::const_reverse_iterator;

    Message(const container_type& args = container_type());
    Message(const Message& that);
    Message(Message&& that);
    Message& operator=(const Message& that);
    Message& operator=(Message&& that);

    inline container_type args()
    {
      return m_args;
    }

    inline const container_type args() const
    {
      return m_args;
    }

    template<class T = value::Base>
    inline std::shared_ptr<T> at(size_type index) const
    {
      return std::static_pointer_cast<T>(m_args[index]);
    }

    inline iterator begin()
    {
      return std::begin(m_args);
    }

    inline const_iterator begin() const
    {
      return std::begin(m_args);
    }

    inline const_iterator cbegin() const
    {
      return std::cbegin(m_args);
    }

    inline iterator end()
    {
      return std::end(m_args);
    }

    inline const_iterator end() const
    {
      return std::end(m_args);
    }

    inline const_iterator cend() const
    {
      return std::cend(m_args);
    }

    inline reverse_iterator rbegin()
    {
      return std::rbegin(m_args);
    }

    inline const_reverse_iterator rbegin() const
    {
      return std::rbegin(m_args);
    }

    inline const_reverse_iterator crbegin() const
    {
      return std::crbegin(m_args);
    }

    inline reverse_iterator rend()
    {
      return std::rend(m_args);
    }

    inline const_reverse_iterator rend() const
    {
      return std::rend(m_args);
    }

    inline const_reverse_iterator crend() const
    {
      return std::crend(m_args);
    }

  private:
    container_type m_args;
  };
}
