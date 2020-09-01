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
#include <snek/ast/stmt/base.hpp>
#include <snek/interpreter.hpp>
#include <snek/lexer.hpp>
#include <snek/parser.hpp>
#include <snek/type/utils.hpp>
#include <snek/value/utils.hpp>

namespace snek
{
  using namespace snek::type::utils;
  using namespace snek::value::utils;

  namespace api::debug { Scope create(const Interpreter&); }
  namespace api::io { Scope create(const Interpreter&); }
  namespace api::list { Scope create(const Interpreter&); }
  namespace api::str { Scope create(const Interpreter&); }

  Interpreter::Interpreter()
    : m_any_type(make_any_type())
    , m_bool_type(make_primitive_type(type::PrimitiveKind::Bool))
    , m_float_type(make_primitive_type(type::PrimitiveKind::Float))
    , m_int_type(make_primitive_type(type::PrimitiveKind::Int))
    , m_num_type(make_primitive_type(type::PrimitiveKind::Num))
    , m_str_type(make_primitive_type(type::PrimitiveKind::Str))
    , m_void_type(make_primitive_type(type::PrimitiveKind::Void))
    , m_null_value(make_null())
    , m_true_value(make_bool(true))
    , m_false_value(make_bool(false))
  {
    m_modules[U"debug"] = api::debug::create(*this);
    m_modules[U"io"] = api::io::create(*this);
    m_modules[U"list"] = api::list::create(*this);
    m_modules[U"str"] = api::str::create(*this);
  }

  Interpreter::Interpreter(const Interpreter& that)
    : m_modules(that.m_modules)
    , m_any_type(that.m_any_type)
    , m_bool_type(that.m_bool_type)
    , m_float_type(that.m_float_type)
    , m_int_type(that.m_int_type)
    , m_num_type(that.m_num_type)
    , m_str_type(that.m_str_type)
    , m_void_type(that.m_void_type)
    , m_null_value(that.m_null_value)
    , m_true_value(that.m_true_value)
    , m_false_value(that.m_false_value) {}

  Interpreter::Interpreter(Interpreter&& that)
    : m_modules(std::move(that.m_modules))
    , m_any_type(std::move(that.m_any_type))
    , m_bool_type(std::move(that.m_bool_type))
    , m_float_type(std::move(that.m_float_type))
    , m_int_type(std::move(that.m_int_type))
    , m_num_type(std::move(that.m_num_type))
    , m_str_type(std::move(that.m_str_type))
    , m_void_type(std::move(that.m_void_type))
    , m_null_value(std::move(that.m_null_value))
    , m_true_value(std::move(that.m_true_value))
    , m_false_value(std::move(that.m_false_value)) {}

  Interpreter&
  Interpreter::operator=(const Interpreter& that)
  {
    m_modules = that.m_modules;
    m_any_type = that.m_any_type;
    m_bool_type = that.m_bool_type;
    m_float_type = that.m_float_type;
    m_int_type = that.m_int_type;
    m_num_type = that.m_num_type;
    m_str_type = that.m_str_type;
    m_void_type = that.m_void_type;
    m_null_value = that.m_null_value;
    m_true_value = that.m_true_value;
    m_false_value = that.m_false_value;

    return *this;
  }

  Interpreter&
  Interpreter::operator=(Interpreter&& that)
  {
    m_modules = std::move(that.m_modules);
    m_any_type = std::move(that.m_any_type);
    m_bool_type = std::move(that.m_bool_type);
    m_float_type = std::move(that.m_float_type);
    m_int_type = std::move(that.m_int_type);
    m_num_type = std::move(that.m_num_type);
    m_str_type = std::move(that.m_str_type);
    m_void_type = std::move(that.m_void_type);
    m_null_value = std::move(that.m_null_value);
    m_true_value = std::move(that.m_true_value);
    m_false_value = std::move(that.m_false_value);

    return *this;
  }

  static parser::result_type
  lex_and_parse(
    const std::u32string& source,
    const std::u32string& file,
    int line,
    int column
  )
  {
    const auto tokens = lexer::lex(source, file, line, column);

    if (!tokens)
    {
      return parser::result_type::error(tokens.error());
    }

    return parser::parse(tokens.value());
  }

  std::optional<Error>
  Interpreter::exec(
    const std::u32string& source,
    const std::u32string& file,
    int line,
    int column
  )
  {
    const auto nodes = lex_and_parse(source, file, line, column);
    Scope scope;
    ast::stmt::ExecContext context;

    if (!nodes)
    {
      return nodes.error();
    }
    for (const auto& node : nodes.value())
    {
      node->exec(*this, scope, context);
      if (context.error())
      {
        return context.error();
      }
      // TODO: Handle dangling break, continue and return.
    }

    return std::nullopt;
  }

  Interpreter::eval_result_type
  Interpreter::eval(
    const std::u32string& source,
    Scope& scope,
    const std::u32string& file,
    int line,
    int column
  )
  {
    const auto nodes = lex_and_parse(source, file, line, column);
    ast::stmt::ExecContext context;

    if (!nodes)
    {
      return eval_result_type::error(nodes.error());
    }
    for (const auto& node : nodes.value())
    {
      node->exec(*this, scope, context);
      if (context.error())
      {
        return eval_result_type::error(*context.error());
      }
      // TODO: Handle dangling break, continue and return.
    }

    return eval_result_type::ok(
      context.value() ? context.value() : m_null_value
    );
  }

  std::optional<const Scope>
  Interpreter::import_module(const std::u32string& path)
  {
    const auto module = m_modules.find(path);

    if (module != std::end(m_modules))
    {
      return module->second;
    }

    return std::nullopt;
  }
}
