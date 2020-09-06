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
#include <snek/ast/type/builtin.hpp>
#include <snek/interpreter.hpp>

namespace snek::ast::type
{
  static const std::unordered_map<BuiltinKind, std::u32string> mapping =
  {
    { BuiltinKind::Any, U"Any" },
    { BuiltinKind::Bin, U"Bin" },
    { BuiltinKind::Bool, U"Bool" },
    { BuiltinKind::Float, U"Float" },
    { BuiltinKind::Int, U"Int" },
    { BuiltinKind::Num, U"Num" },
    { BuiltinKind::Str, U"Str" },
    { BuiltinKind::Void, U"Void" },
  };

  Builtin::Builtin(const Position& position, BuiltinKind builtin_kind)
    : Base(position)
    , m_builtin_kind(builtin_kind) {}

  std::u32string
  Builtin::to_string() const
  {
    const auto entry = mapping.find(m_builtin_kind);

    return entry != std::end(mapping) ? entry->second : U"Any";
  }

  Base::result_type
  Builtin::eval(const Interpreter& interpreter, const Scope&) const
  {
    switch (m_builtin_kind)
    {
      case BuiltinKind::Any:
        return result_type::ok(interpreter.any_type());

      case BuiltinKind::Bin:
        return result_type::ok(interpreter.bin_type());

      case BuiltinKind::Bool:
        return result_type::ok(interpreter.bool_type());

      case BuiltinKind::Float:
        return result_type::ok(interpreter.float_type());

      case BuiltinKind::Int:
        return result_type::ok(interpreter.int_type());

      case BuiltinKind::Num:
        return result_type::ok(interpreter.num_type());

      case BuiltinKind::Str:
        return result_type::ok(interpreter.str_type());

      case BuiltinKind::Void:
        return result_type::ok(interpreter.void_type());
    }

    return result_type::error({
      position(),
      U"Unrecognized builtin type."
    });
  }
}
