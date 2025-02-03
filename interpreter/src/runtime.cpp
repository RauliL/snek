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
#include "snek/error.hpp"
#include "snek/interpreter/execute.hpp"
#include "snek/interpreter/jump.hpp"
#include "snek/interpreter/runtime.hpp"
#include "snek/parser/statement.hpp"

namespace snek::interpreter
{
  using prototype_type = value::Record::container_type;
  using prototype_constructor = void(*)(const Runtime*, prototype_type&);

  namespace prototype
  {
    void MakeBoolean(const Runtime*, prototype_type&);
    void MakeFloat(const Runtime*, prototype_type&);
    void MakeInt(const Runtime*, prototype_type&);
    void MakeList(const Runtime*, prototype_type&);
    void MakeNumber(const Runtime*, prototype_type&);
    void MakeObject(const Runtime*, prototype_type&);
    void MakeRecord(const Runtime*, prototype_type&);
    void MakeString(const Runtime*, prototype_type&);
  }

  static inline value::ptr
  MakePrototype(
    const Runtime* runtime,
    const value::ptr& parent,
    prototype_constructor constructor = nullptr
  )
  {
    value::Record::container_type fields;

    fields[U"[[Prototype]]"] = parent;
    if (constructor)
    {
      constructor(runtime, fields);
    }

    return std::make_shared<value::Record>(fields);
  }

  Runtime::Runtime()
    : m_any_type(std::make_shared<type::Any>())
    , m_boolean_type(std::make_shared<type::Builtin>(type::BuiltinKind::Boolean))
    , m_float_type(std::make_shared<type::Builtin>(type::BuiltinKind::Float))
    , m_function_type(std::make_shared<type::Builtin>(type::BuiltinKind::Function))
    , m_int_type(std::make_shared<type::Builtin>(type::BuiltinKind::Int))
    , m_list_type(std::make_shared<type::Builtin>(type::BuiltinKind::List))
    , m_number_type(std::make_shared<type::Builtin>(type::BuiltinKind::Number))
    , m_record_type(std::make_shared<type::Builtin>(type::BuiltinKind::Record))
    , m_string_type(std::make_shared<type::Builtin>(type::BuiltinKind::String))
    , m_void_type(std::make_shared<type::Builtin>(type::BuiltinKind::Void))

    , m_object_prototype(MakePrototype(
        this,
        nullptr,
        prototype::MakeObject
      ))
    , m_number_prototype(MakePrototype(
        this,
        m_object_prototype,
        prototype::MakeNumber
      ))
    , m_boolean_prototype(MakePrototype(
        this,
        m_object_prototype,
        prototype::MakeBoolean
      ))
    , m_float_prototype(MakePrototype(
        this,
        m_number_prototype,
        prototype::MakeFloat
      ))
    , m_function_prototype(MakePrototype(this, m_object_prototype))
    , m_int_prototype(MakePrototype(
        this,
        m_number_prototype,
        prototype::MakeInt
      ))
    , m_list_prototype(MakePrototype(
        this,
        m_object_prototype,
        prototype::MakeList
      ))
    , m_record_prototype(MakePrototype(
        this,
        m_object_prototype,
        prototype::MakeRecord
      ))
    , m_string_prototype(MakePrototype(
        this,
        m_object_prototype,
        prototype::MakeString
      ))

    , m_root_scope(Scope::MakeRootScope(this)) {}

  value::ptr
  Runtime::RunScript(
    const Scope::ptr& scope,
    const std::string& source,
    const std::u32string& filename,
    int line,
    int column
  )
  {
    parser::Lexer lexer(
      std::begin(source),
      std::end(source),
      filename,
      line,
      column
    );
    value::ptr value;

    try
    {
      while (!lexer.PeekToken(parser::Token::Kind::Eof))
      {
        value = ExecuteStatement(
          *this,
          scope,
          parser::statement::Parse(lexer, true)
        );
      }
    }
    catch (const Jump& jump)
    {
      throw Error{
        jump.position(),
        U"Unexpected `" +
        parser::statement::Jump::ToString(jump.kind())
        + U"'."
      };
    }

    return value;
  }
}
