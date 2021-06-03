/*
 * Copyright (c) 2020-2021, Rauli Laine
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
#include <snek/value/utils.hpp>

namespace snek::api::record
{
  using result_type = value::Func::result_type;
  using value::utils::make_str;
  using value::utils::make_list;

  /**
   * Creates an list containing all keys of an record.
   *
   *     keys({ foo: "bar" }) == ["foo"]
   */
  static result_type
  func_keys(Interpreter& interpreter, const Message& message)
  {
    const auto& input = message.at<value::Record>(0);
    value::List::container_type output;

    output.reserve(input->fields().size());
    for (const auto& field : *input)
    {
      output.push_back(make_str(field.first));
    }

    return result_type::ok(make_list(output));
  }

  /**
   * Creates an list containing all values of an record.
   *
   *     values({ foo: "bar" }) == ["bar"]
   */
  static result_type
  func_values(Interpreter& interpreter, const Message& message)
  {
    const auto& input = message.at<value::Record>(0);
    value::List::container_type output;

    output.reserve(input->fields().size());
    for (const auto& field : *input)
    {
      output.push_back(field.second);
    }

    return result_type::ok(make_list(output));
  }

  Scope
  create(const Interpreter& interpreter)
  {
    using namespace snek::type::utils;
    const auto& record_type = interpreter.record_type();

    return create_module({
      {
        U"keys",
        func_keys,
        { Parameter(U"input", record_type) },
        make_list_type(interpreter.str_type()),
      },
      {
        U"values",
        func_values,
        { Parameter(U"input", record_type) },
        // TODO: Implement generics at some point.
        make_list_type(interpreter.any_type()),
      },
    });
  }
}
