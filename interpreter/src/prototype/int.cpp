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
#include <climits>
#include <random>

#include "snek/interpreter/runtime.hpp"

namespace snek::interpreter::prototype
{
  static inline std::int64_t AsInt(const value::ptr& value)
  {
    return static_cast<const value::Int*>(value.get())->value();
  }

  /**
   * Int#random(min: Int | null = null, max: Int | null = null) => Int
   *
   * Generates random integer number. Optional minimum and maximum values can
   * be given.
   */
  static value::ptr
  Random(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    thread_local static std::random_device device;
    thread_local static std::mt19937 generator(device());

    const auto min =
      value::IsNull(arguments[0])
        ? INT64_MIN
        : AsInt(arguments[0]);
    const auto max =
      value::IsNull(arguments[1])
        ? INT64_MAX
        : AsInt(arguments[1]);
    std::uniform_int_distribution<std::int64_t> d(min ,max);

    return runtime.MakeInt(d(generator));
  }

  void
  MakeInt(const Runtime* runtime, value::Record::container_type& fields)
  {
    const auto nullable_int = std::make_shared<type::Union>(
      type::Union::container_type{
        runtime->int_type(),
        runtime->void_type()
      }
    );
    const auto null_expression = std::make_shared<parser::expression::Null>(
      Position{ U"", 0, 0 }
    );

    fields[U"random"] = value::Function::MakeNative(
      {
        { U"min", nullable_int, null_expression },
        { U"max", nullable_int, null_expression },
      },
      runtime->int_type(),
      Random
    );
  }
}


