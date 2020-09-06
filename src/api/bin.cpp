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
#include <snek/api.hpp>
#include <snek/interpreter.hpp>
#include <snek/type/utils.hpp>
#include <snek/value/bin.hpp>
#include <snek/value/utils.hpp>

namespace snek::api::bin
{
  using namespace snek::value::utils;
  using result_type = value::Func::result_type;

  /**
   * Determines whether binary data is empty.
   *
   *     isEmpty(b"") == true
   *     isEmpty(b"foo") == false
   */
  static result_type
  func_isEmpty(Interpreter& interpreter, const Message& message)
  {
    const auto& input = message.at<value::Bin>(0);

    return result_type::ok(interpreter.bool_value(input->value().empty()));
  }

  /**
   * Returns length of binary data.
   *
   *     length(b"hello") == 5
   */
  static result_type
  func_length(Interpreter& interpreter, const Message& message)
  {
    const auto& input = message.at<value::Bin>(0);

    return result_type::ok(make_int(input->value().length()));
  }

  /**
   * Reverse binary data.
   *
   *     reverse(b"foo") == b"oof"
   */
  static result_type
  func_reverse(Interpreter& interpreter, const Message& message)
  {
    const auto& input = message.at<value::Bin>(0);

    return result_type::ok(make_str(input->rbegin(), input->rend()));
  }

  /**
   * Repeats binary data given number of times.
   *
   *      repeat(3, b"ha") == b"hahaha"
   */
  static result_type
  func_repeat(Interpreter& interpreter, const Message& message)
  {
    const auto& times = message.at<value::Int>(0);
    const auto& input = message.at<value::Bin>(1);
    std::string result;

    result.reserve(times->value());
    for (int i = 0; i < times->value(); ++i)
    {
      result += input->value();
    }

    return result_type::ok(std::make_shared<value::Bin>(result));
  }

  /**
   * Concatenates multiple binary datas into one.
   *
   *     concat([b"foo", b"bar", b"baz"]) == b"foobarbaz"
   */
  static result_type
  func_concat(Interpreter& interpreter, const Message& message)
  {
    const auto& list = message.at<value::List>(0);
    std::string result;

    for (const auto& element : *list)
    {
      result += std::static_pointer_cast<value::Bin>(element)->value();
    }

    return result_type::ok(std::make_shared<value::Bin>(result));
  }

  Scope
  create(const Interpreter& interpreter)
  {
    using namespace snek::type::utils;
    const auto& bin_type = interpreter.bin_type();
    const auto& int_type = interpreter.int_type();

    return create_module({
      {
        U"isEmpty",
        func_isEmpty,
        { Parameter(U"input", bin_type) },
        interpreter.bool_type()
      },
      {
        U"length",
        func_length,
        { Parameter(U"input", bin_type) },
        int_type
      },
      {
        U"reverse",
        func_reverse,
        { Parameter(U"input", bin_type) },
        bin_type
      },
      {
        U"repeat",
        func_repeat,
        {
          Parameter(U"times", int_type),
          Parameter(U"input", bin_type),
        },
        bin_type
      },
      {
        U"concat",
        func_concat,
        { Parameter(U"list", make_list_type(bin_type)) },
        bin_type
      },
    });
  }
}
