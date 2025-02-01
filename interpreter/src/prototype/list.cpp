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
#include "snek/error.hpp"
#include "snek/interpreter/runtime.hpp"

namespace snek::interpreter::prototype
{
  static inline const value::List*
  AsList(const value::ptr& value)
  {
    return static_cast<const value::List*>(value.get());
  }

  /**
   * List#size(this: List) => Int
   *
   * Returns size of the list.
   */
  static value::ptr
  Size(
    Runtime&,
    const std::vector<value::ptr>& arguments
  )
  {
    return std::make_shared<value::Int>(AsList(arguments[0])->GetSize());
  }

  /**
   * List#[](this: List, index: Number) => any
   *
   * Returns list element from given index. If the index is out of bounds, an
   * exception will be thrown.
   */
  static value::ptr
  At(
    Runtime&,
    const std::vector<value::ptr>& arguments
  )
  {
    const auto list = AsList(arguments[0]);
    const auto size = list->GetSize();
    auto index = std::static_pointer_cast<value::Number>(
      arguments[1]
    )->ToInt();

    if (index < 0)
    {
      index += size;
    }
    if (
      !size ||
      index < 0 ||
      index >= static_cast<value::Number::int_type>(size)
    )
    {
      throw Error{ std::nullopt, U"List index out of bounds." };
    }

    return list->At(index);
  }

  /**
   * List#+(this: List, other: List) => List
   *
   * Concatenates contents of two lists together.
   */
  static value::ptr
  Concat(
    Runtime&,
    const std::vector<value::ptr>& arguments
  )
  {
    return value::List::Concat(
      std::static_pointer_cast<value::List>(arguments[0]),
      std::static_pointer_cast<value::List>(arguments[1])
    );
  }

  void
  MakeList(const Runtime* runtime, value::Record::container_type& fields)
  {
    fields[U"size"] = value::Function::MakeNative(
      {
        Parameter(U"this", runtime->list_type())
      },
      runtime->int_type(),
      Size
    );

    fields[U"[]"] = value::Function::MakeNative(
      {
        Parameter(U"this", runtime->list_type()),
        Parameter(U"index", runtime->number_type())
      },
      runtime->any_type(),
      At
    );
    fields[U"+"] = value::Function::MakeNative(
      {
        Parameter(U"this", runtime->list_type()),
        Parameter(U"index", runtime->list_type())
      },
      runtime->list_type(),
      Concat
    );
  }
}
