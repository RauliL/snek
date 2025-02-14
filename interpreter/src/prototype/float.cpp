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
#include <cerrno>
#include <cfloat>
#include <cstdlib>
#include <random>

#include <peelo/unicode/encoding/utf8.hpp>

#include "snek/interpreter/error.hpp"
#include "snek/interpreter/runtime.hpp"

namespace snek::interpreter::prototype
{
  static inline double
  AsFloat(const value::ptr& value)
  {
    return static_cast<const value::Number*>(value.get())->ToFloat();
  }

  /**
   * Float#parse(input: String) => Float
   *
   * Parses given string as floating point decimal and returns result.
   */
  static value::ptr
  Parse(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    using peelo::unicode::encoding::utf8::encode;

    const auto input = encode(value::ToString(arguments[0]));
    // TODO: Implement Unicode version of std::strtod.
    const auto result = std::strtod(input.c_str(), nullptr);

    if (errno == ERANGE)
    {
      throw runtime.MakeError(U"Float out of range.");
    }

    return std::make_shared<value::Float>(result);
  }

  /**
   * Float#random(min: Float | null = null, max: Float | null = null) => Float
   *
   * Generates random floating point decimal value. Optional minimum and
   * maximum values can be given.
   */
  static value::ptr
  Random(Runtime&, const std::vector<value::ptr>& arguments)
  {
    thread_local static std::random_device device;
    thread_local static std::mt19937 generator(device());

    const auto min =
      value::IsNull(arguments[0])
        ? DBL_MIN
        : AsFloat(arguments[0]);
    const auto max =
      value::IsNull(arguments[1])
        ? DBL_MAX
        : AsFloat(arguments[1]);
    std::uniform_real_distribution<double> d(min ,max);

    return std::make_shared<value::Float>(d(generator));
  }

  void
  MakeFloat(
    const Runtime* runtime,
    std::unordered_map<std::u32string, value::ptr>& fields
  )
  {
    const auto optional_float = type::MakeOptional(runtime->float_type());
    const auto null_expression = std::make_shared<parser::expression::Null>(
      std::nullopt
    );

    fields[U"parse"] = value::Function::MakeNative(
      { { U"input", runtime->string_type() } },
      runtime->float_type(),
      Parse
    );
    fields[U"random"] = value::Function::MakeNative(
      {
        { U"min", optional_float, null_expression },
        { U"max", optional_float, null_expression },
      },
      runtime->float_type(),
      Random
    );
  }
}

