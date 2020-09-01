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
#include <snek/message.hpp>
#include <snek/parameter.hpp>
#include <snek/type/base.hpp>

namespace snek
{
  Message::Message(const container_type& arguments)
    : m_arguments(arguments) {}

  Message::Message(const Message& that)
    : m_arguments(that.m_arguments) {}

  Message::Message(Message&& that)
    : m_arguments(std::move(that.m_arguments)) {}

  Message&
  Message::operator=(const Message& that)
  {
    m_arguments = that.m_arguments;

    return *this;
  }

  Message&
  Message::operator=(Message&& that)
  {
    m_arguments = std::move(that.m_arguments);

    return *this;
  }

  Message::result_type
  Message::create(
    const std::vector<Parameter>& parameters,
    const std::vector<value::Ptr>& arguments,
    const Interpreter& interpreter,
    const std::optional<ast::Position>& position
  )
  {
    container_type container;

    if (parameters.size() > arguments.size())
    {
      return result_type::error({
        position,
        U"Not enough arguments."
      });
    }
    for (std::size_t i = 0; i < parameters.size(); ++i)
    {
      const auto& parameter = parameters[i];
      const auto& argument = arguments[i];

      if (!parameter.type()->matches(argument))
      {
        return result_type::error({
          position,
          argument->type(interpreter)->to_string() +
          U" cannot be assigned to " +
          parameter.type()->to_string() +
          U"."
        });
      }
      container[parameter.name()] = argument;
    }

    return result_type::ok(Message(container));
  }
}
