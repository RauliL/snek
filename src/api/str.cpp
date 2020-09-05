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
#include <peelo/unicode/ctype/tolower.hpp>
#include <peelo/unicode/ctype/toupper.hpp>

#include <snek/api.hpp>
#include <snek/interpreter.hpp>
#include <snek/type/utils.hpp>
#include <snek/value/utils.hpp>

namespace snek::api::str
{
  using namespace snek::value::utils;
  using result_type = value::Func::result_type;

  /**
   * Determines whether a string is empty.
   *
   *     isEmpty("") == true
   *     isEmpty("foo") == false
   */
  static result_type
  func_isEmpty(Interpreter& interpreter, const Message& message)
  {
    const auto& input = message.at<value::Str>(0);

    return result_type::ok(interpreter.bool_value(input->value().empty()));
  }

  /**
   * Returns length of a string.
   *
   *     length("hello") == 5
   */
  static result_type
  func_length(Interpreter& interpreter, const Message& message)
  {
    const auto& input = message.at<value::Str>(0);

    return result_type::ok(make_int(input->value().length()));
  }

  /**
   * Reverse a string.
   *
   *     reverse("foo") == "oof"
   */
  static result_type
  func_reverse(Interpreter& interpreter, const Message& message)
  {
    const auto& input = message.at<value::Str>(0);

    return result_type::ok(make_str(input->rbegin(), input->rend()));
  }

  /**
   * Repeats a string given number of times.
   *
   *      repeat(3, "ha") == "hahaha"
   */
  static result_type
  func_repeat(Interpreter& interpreter, const Message& message)
  {
    const auto& times = message.at<value::Int>(0);
    const auto& input = message.at<value::Str>(1);
    std::u32string result;

    result.reserve(times->value());
    for (int i = 0; i < times->value(); ++i)
    {
      result += input->value();
    }

    return result_type::ok(make_str(result));
  }

  /**
   * Concatenates multiple strings into one.
   *
   *     concat(["foo", "bar", "baz"]) == "foobarbaz"
   */
  static result_type
  func_concat(Interpreter& interpreter, const Message& message)
  {
    const auto& list = message.at<value::List>(0);
    std::u32string result;

    for (const auto& element : *list)
    {
      result += std::static_pointer_cast<value::Str>(element)->value();
    }

    return result_type::ok(make_str(result));
  }

  /**
   * Converts string into upper case.
   *
   *    toUpper("foo") == "FOO"
   */
  static result_type
  func_toUpper(Interpreter& interpreter, const Message& message)
  {
    const auto& input = message.at<value::Str>(0);
    std::u32string result;

    result.reserve(input->value().length());
    for (const auto& c : *input)
    {
      result.append(1, peelo::unicode::ctype::toupper(c));
    }

    return result_type::ok(make_str(result));
  }

  /**
   * Converts string into lower case.
   *
   *    toLower("FOO") == "foo"
   */
  static result_type
  func_toLower(Interpreter& interpreter, const Message& message)
  {
    const auto& input = message.at<value::Str>(0);
    std::u32string result;

    result.reserve(input->value().length());
    for (const auto& c : *input)
    {
      result.append(1, peelo::unicode::ctype::tolower(c));
    }

    return result_type::ok(make_str(result));
  }

  Scope
  create(const Interpreter& interpreter)
  {
    using namespace snek::type::utils;
    const auto& str_type = interpreter.str_type();
    const auto& int_type = interpreter.int_type();

    return create_module({
      {
        U"isEmpty",
        func_isEmpty,
        { Parameter(U"input", str_type) },
        interpreter.bool_type()
      },
      {
        U"length",
        func_length,
        { Parameter(U"input", str_type) },
        int_type
      },
      {
        U"reverse",
        func_reverse,
        { Parameter(U"input", str_type) },
        str_type
      },
      {
        U"repeat",
        func_repeat,
        {
          Parameter(U"times", int_type),
          Parameter(U"input", str_type),
        },
        str_type
      },
      {
        U"concat",
        func_concat,
        { Parameter(U"list", make_list_type(str_type)) },
        str_type
      },
      {
        U"toUpper",
        func_toUpper,
        { Parameter(U"input", str_type) },
        str_type
      },
      {
        U"toLower",
        func_toLower,
        { Parameter(U"input", str_type) },
        str_type
      },
    });
  }
}
