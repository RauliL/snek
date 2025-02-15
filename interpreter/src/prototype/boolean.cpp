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
#include <random>

#include "snek/interpreter/runtime.hpp"

namespace snek::interpreter::prototype
{
  /**
   * Boolean#random(distribution: Float = 0.5) => Boolean
   *
   * Generates random boolean value.
   */
  static value::ptr
  Random(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    thread_local static std::random_device device;
    thread_local static std::mt19937 generator(device());

    std::bernoulli_distribution d(
      static_cast<const value::Float*>(arguments[0].get())->value
    );

    return runtime.MakeBoolean(d(generator));
  }

  void
  MakeBoolean(
    const Runtime* runtime,
    std::unordered_map<std::u32string, value::ptr>& fields
  )
  {
    fields[U"random"] = value::Function::MakeNative(
      {
        {
          U"distribution",
          runtime->float_type(),
          std::make_shared<parser::expression::Float>(std::nullopt, 0.5)
        }
      },
      runtime->boolean_type(),
      Random
    );
  }
}
