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
#include <snek/interpreter.hpp>
#include <snek/message.hpp>
#include <snek/value/func.hpp>
#include <snek/value/utils.hpp>

namespace snek::api::debug
{
  using namespace snek::value::utils;
  using result_type = value::Func::result_type;

  /**
   * Converts any kind of value into a string.
   *
   *     toString(1) == "1"
   *     toString([1, 2]) == "[1, 2]"
   *     toString("foo") == "\"foo\""
   */
  static result_type
  func_toString(Interpreter& interpreter, const Message& message)
  {
    return result_type::ok(make_str(
      message.get<value::Base>(U"input")->to_string()
    ));
  }

  Scope
  create(const Interpreter& interpreter)
  {
    return Scope(
      {},
      {
        {
          U"toString",
          {
            {
              make_func(
                {
                  Parameter(
                    U"input",
                    interpreter.any_type()
                  )
                },
                func_toString,
                interpreter.str_type()
              )
            },
            true
          }
        },
      }
    );
  }
}
