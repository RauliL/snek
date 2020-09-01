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
#include <snek/parameter.hpp>

namespace snek
{
  Parameter::Parameter(
    const std::u32string& name,
    const type::Ptr& type
  )
    : m_name(name)
    , m_type(type) {}

  Parameter::Parameter(const Parameter& that)
    : m_name(that.m_name)
    , m_type(that.m_type) {}

  Parameter::Parameter(Parameter&& that)
    : m_name(std::move(that.m_name))
    , m_type(std::move(that.m_type)) {}

  Parameter&
  Parameter::operator=(const Parameter& that)
  {
    m_name = that.m_name;
    m_type = that.m_type;

    return *this;
  }

  Parameter&
  Parameter::operator=(Parameter&& that)
  {
    m_name = std::move(that.m_name);
    m_type = std::move(that.m_type);

    return *this;
  }

  std::u32string
  Parameter::to_string() const
  {
    return m_name + U": " + m_type->to_string();
  }
}
