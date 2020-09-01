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
#include <snek/ast/stmt/base.hpp>
#include <snek/interpreter.hpp>

namespace snek::ast::stmt
{
  ExecContext::ExecContext() {}

  ExecContext::ExecContext(const ExecContext& that)
    : m_jump(that.m_jump)
    , m_error(that.m_error)
    , m_value(that.m_value) {}

  ExecContext::ExecContext(ExecContext&& that)
    : m_jump(std::move(that.m_jump))
    , m_error(std::move(that.m_error))
    , m_value(std::move(that.m_value)) {}

  ExecContext&
  ExecContext::operator=(const ExecContext& that)
  {
    m_jump = that.m_jump;
    m_error = that.m_error;
    m_value = that.m_value;

    return *this;
  }

  ExecContext&
  ExecContext::operator=(ExecContext&& that)
  {
    m_jump = std::move(that.m_jump);
    m_error = std::move(that.m_error);
    m_value = std::move(that.m_value);

    return *this;
  }

  Base::Base(const Position& position)
    : Node(position) {}
}
