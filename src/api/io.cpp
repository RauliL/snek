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
#include <snek/value/utils.hpp>
#include <snek/type/utils.hpp>

namespace snek::api::io
{
  using result_type = value::Func::result_type;

  static result_type
  func_print(Interpreter& interpreter, const Message& message)
  {
    using peelo::unicode::encoding::utf8::encode;
    const auto input = message.at<value::Str>(0);

    std::cout << encode(input->value()) << std::endl;

    return result_type::ok(interpreter.null_value());
  }

  Scope
  create(const Interpreter& interpreter)
  {
    using namespace snek::type::utils;
    using namespace snek::value::utils;

    return Scope(
      {},
      {
        {
          U"print",
          {
            make_func(
              { Parameter(U"input", interpreter.str_type()) },
              func_print,
              interpreter.void_type()
            ),
            true
          }
        },
      }
    );
  }
}
