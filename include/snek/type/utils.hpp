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

#include <snek/type/any.hpp>
#include <snek/type/func.hpp>
#include <snek/type/list.hpp>
#include <snek/type/primitive.hpp>
#include <snek/type/record.hpp>
#include <snek/type/union.hpp>

namespace snek::type::utils
{
  inline AnyPtr
  make_any_type()
  {
    return std::make_shared<type::Any>();
  }

  inline PrimitivePtr
  make_primitive_type(type::PrimitiveKind kind)
  {
    return std::make_shared<type::Primitive>(kind);
  }

  inline ListPtr
  make_list_type(const Ptr& element_type)
  {
    return std::make_shared<type::List>(element_type);
  }

  inline FuncPtr
  make_func_type(
    const std::vector<Parameter>& parameters,
    const Ptr& return_type
  )
  {
    return std::make_shared<type::Func>(parameters, return_type);
  }

  inline RecordPtr
  make_record_type(const type::Record::container_type& fields)
  {
    return std::make_shared<type::Record>(fields);
  }

  inline UnionPtr
  make_union_type(const type::Union::container_type& types)
  {
    return std::make_shared<type::Union>(types);
  }
}
