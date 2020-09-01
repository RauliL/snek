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

#include <unordered_map>
#include <vector>

#include <peelo/result.hpp>

#include <snek/error.hpp>
#include <snek/value/base.hpp>

namespace snek
{
  class Interpreter;
  class Parameter;

  namespace ast { class Position; }

  class Message
  {
  public:
    using key_type = std::u32string;
    using mapped_type = value::Ptr;
    using container_type = std::unordered_map<key_type, mapped_type>;
    using value_type = container_type::value_type;
    using iterator = container_type::iterator;
    using const_iterator = container_type::const_iterator;
    using result_type = peelo::result<Message, Error>;

    Message(const container_type& arguments = container_type());
    Message(const Message& that);
    Message(Message&& that);
    Message& operator=(const Message& that);
    Message& operator=(Message&& that);

    static result_type create(
      const std::vector<Parameter>& parameters,
      const std::vector<value::Ptr>& arguments,
      const Interpreter& interpreter,
      const std::optional<ast::Position>& position = std::nullopt
    );

    inline container_type& arguments()
    {
      return m_arguments;
    }

    inline const container_type& arguments() const
    {
      return m_arguments;
    }

    template<class T>
    inline std::shared_ptr<T> get(const std::u32string& name) const
    {
      return std::static_pointer_cast<T>(m_arguments.find(name)->second);
    }

    inline iterator begin()
    {
      return std::begin(m_arguments);
    }

    inline const_iterator begin() const
    {
      return std::begin(m_arguments);
    }

    inline iterator end()
    {
      return std::end(m_arguments);
    }

    inline const_iterator end() const
    {
      return std::end(m_arguments);
    }

  private:
    container_type m_arguments;
  };
}
