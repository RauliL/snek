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
#include <snek/value/bool.hpp>
#include <snek/value/float.hpp>
#include <snek/value/func.hpp>
#include <snek/value/int.hpp>
#include <snek/value/list.hpp>
#include <snek/value/record.hpp>
#include <snek/value/str.hpp>

namespace snek::value::utils
{
  inline value::FloatPtr
  make_float(value::Float::value_type value)
  {
    return std::make_shared<value::Float>(value);
  }

  inline value::IntPtr
  make_int(value::Int::value_type value)
  {
    return std::make_shared<value::Int>(value);
  }

  inline value::ListPtr
  make_list(const value::List::container_type& elements)
  {
    return std::make_shared<value::List>(elements);
  }

  template<class IteratorT>
  inline value::ListPtr
  make_list(const IteratorT& begin, const IteratorT& end)
  {
    return std::make_shared<value::List>(value::List::container_type(
      begin,
      end
    ));
  }

  inline value::RecordPtr
  make_record(const value::Record::container_type& fields)
  {
    return std::make_shared<value::Record>(fields);
  }

  inline value::StrPtr
  make_str(value::Str::const_reference value)
  {
    return std::make_shared<value::Str>(value);
  }

  template<class IteratorT>
  inline value::StrPtr
  make_str(const IteratorT& begin, const IteratorT& end)
  {
    return std::make_shared<value::Str>(value::Str::value_type(begin, end));
  }

  inline value::FuncPtr
  make_func(
    const std::vector<Parameter>& parameters,
    value::Func::callback_type callback,
    const std::optional<type::Ptr>& return_type = std::nullopt
  )
  {
    return std::make_shared<value::Func>(
      parameters,
      value::Func::body_type(callback),
      return_type
    );
  }

  inline value::FuncPtr
  make_func(
    const std::vector<Parameter>& parameters,
    const std::shared_ptr<ast::stmt::Base>& body,
    const std::optional<type::Ptr>& return_type = std::nullopt,
    const std::optional<Scope>& enclosing_scope = std::nullopt
  )
  {
    return std::make_shared<value::Func>(
      parameters,
      value::Func::body_type(body),
      return_type,
      enclosing_scope
    );
  }
}
