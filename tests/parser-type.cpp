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
#include <lest/lest.hpp>

#include <snek/ast/type.hpp>
#include <snek/lexer.hpp>
#include <snek/parser.hpp>

using namespace snek;

static parser::type::result_type
parse(const std::u32string& source)
{
  const auto tokens = lexer::lex(source).value();
  parser::State state = { std::begin(tokens), std::end(tokens) };

  return parser::type::parse(state);
}

static const lest::test specification[] =
{
  CASE("End of input should fail")
  {
    const auto result = parse(U"");

    EXPECT(!result);
  },

  CASE("Unrecognized type should fail")
  {
    const auto result = parse(U")");

    EXPECT(!result);
  },

  CASE("Named type")
  {
    const auto result = parse(U"test");
    std::shared_ptr<ast::type::Named> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::Named);
    type = std::static_pointer_cast<ast::type::Named>(result.value());
    EXPECT(!type->name().compare(U"test"));
  },

  CASE("Any")
  {
    const auto result = parse(U"Any");
    std::shared_ptr<ast::type::Builtin> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::Builtin);
    type = std::static_pointer_cast<ast::type::Builtin>(result.value());
    EXPECT(type->builtin_kind() == ast::type::BuiltinKind::Any);
  },

  CASE("Bool")
  {
    const auto result = parse(U"Bool");
    std::shared_ptr<ast::type::Builtin> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::Builtin);
    type = std::static_pointer_cast<ast::type::Builtin>(result.value());
    EXPECT(type->builtin_kind() == ast::type::BuiltinKind::Bool);
  },

  CASE("Float")
  {
    const auto result = parse(U"Float");
    std::shared_ptr<ast::type::Builtin> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::Builtin);
    type = std::static_pointer_cast<ast::type::Builtin>(result.value());
    EXPECT(type->builtin_kind() == ast::type::BuiltinKind::Float);
  },

  CASE("Int")
  {
    const auto result = parse(U"Int");
    std::shared_ptr<ast::type::Builtin> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::Builtin);
    type = std::static_pointer_cast<ast::type::Builtin>(result.value());
    EXPECT(type->builtin_kind() == ast::type::BuiltinKind::Int);
  },

  CASE("Num")
  {
    const auto result = parse(U"Num");
    std::shared_ptr<ast::type::Builtin> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::Builtin);
    type = std::static_pointer_cast<ast::type::Builtin>(result.value());
    EXPECT(type->builtin_kind() == ast::type::BuiltinKind::Num);
  },

  CASE("Str")
  {
    const auto result = parse(U"Str");
    std::shared_ptr<ast::type::Builtin> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::Builtin);
    type = std::static_pointer_cast<ast::type::Builtin>(result.value());
    EXPECT(type->builtin_kind() == ast::type::BuiltinKind::Str);
  },

  CASE("Void")
  {
    const auto result = parse(U"Void");
    std::shared_ptr<ast::type::Builtin> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::Builtin);
    type = std::static_pointer_cast<ast::type::Builtin>(result.value());
    EXPECT(type->builtin_kind() == ast::type::BuiltinKind::Void);
  },

  CASE("Function type (without arguments)")
  {
    const auto result = parse(U"() -> Any");
    std::shared_ptr<ast::type::Func> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::Func);
    type = std::static_pointer_cast<ast::type::Func>(result.value());
    EXPECT(type->parameters().size() == 0);
  },

  CASE("Function type (with one argument)")
  {
    const auto result = parse(U"(a) -> Any");
    std::shared_ptr<ast::type::Func> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::Func);
    type = std::static_pointer_cast<ast::type::Func>(result.value());
    EXPECT(type->parameters().size() == 1);
  },

  CASE("Function type (with multiple arguments)")
  {
    const auto result = parse(U"(a, b, c) -> Any");
    std::shared_ptr<ast::type::Func> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::Func);
    type = std::static_pointer_cast<ast::type::Func>(result.value());
    EXPECT(type->parameters().size() == 3);
  },

  CASE("Function type (with typed arguments)")
  {
    const auto result = parse(U"(a: Int, b: Int) -> Int");
    std::shared_ptr<ast::type::Func> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::Func);
    type = std::static_pointer_cast<ast::type::Func>(result.value());
    EXPECT(type->parameters().size() == 2);
  },

  CASE("Function type (with trailing comma)")
  {
    const auto result = parse(U"(a: Int,) -> Int");
    std::shared_ptr<ast::type::Func> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::Func);
    type = std::static_pointer_cast<ast::type::Func>(result.value());
    EXPECT(type->parameters().size() == 1);
  },

  CASE("Tuple type (empty)")
  {
    const auto result = parse(U"[]");
    std::shared_ptr<ast::type::Tuple> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::Tuple);
    type = std::static_pointer_cast<ast::type::Tuple>(result.value());
    EXPECT(type->types().size() == 0);
  },

  CASE("Tuple type (with one element)")
  {
    const auto result = parse(U"[Int]");
    std::shared_ptr<ast::type::Tuple> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::Tuple);
    type = std::static_pointer_cast<ast::type::Tuple>(result.value());
    EXPECT(type->types().size() == 1);
  },

  CASE("Tuple type (with multiple elements)")
  {
    const auto result = parse(U"[Int, Str]");
    std::shared_ptr<ast::type::Tuple> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::Tuple);
    type = std::static_pointer_cast<ast::type::Tuple>(result.value());
    EXPECT(type->types().size() == 2);
  },

  CASE("Tuple type (with trailing comma)")
  {
    const auto result = parse(U"[Int,]");
    std::shared_ptr<ast::type::Tuple> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::Tuple);
    type = std::static_pointer_cast<ast::type::Tuple>(result.value());
    EXPECT(type->types().size() == 1);
  },

  CASE("Record type (without fields)")
  {
    const auto result = parse(U"{}");
    std::shared_ptr<ast::type::Record> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::Record);
    type = std::static_pointer_cast<ast::type::Record>(result.value());
    EXPECT(type->fields().size() == 0);
  },

  CASE("Record type (with one field)")
  {
    const auto result = parse(U"{ a: Str }");
    std::shared_ptr<ast::type::Record> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::Record);
    type = std::static_pointer_cast<ast::type::Record>(result.value());
    EXPECT(type->fields().size() == 1);
  },

  CASE("Record type (with multiple fields)")
  {
    const auto result = parse(U"{ a: Str, b: Bool }");
    std::shared_ptr<ast::type::Record> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::Record);
    type = std::static_pointer_cast<ast::type::Record>(result.value());
    EXPECT(type->fields().size() == 2);
  },

  CASE("Record type (with trailing comma)")
  {
    const auto result = parse(U"{ a: Str, }");
    std::shared_ptr<ast::type::Record> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::Record);
    type = std::static_pointer_cast<ast::type::Record>(result.value());
    EXPECT(type->fields().size() == 1);
  },

  CASE("String type")
  {
    const auto result = parse(U"\"foo\"");
    //std::shared_ptr<ast::type::Str> type;

    EXPECT(result);
    //EXPECT(result.value()->kind() == ast::type::Kind::Str);
    //type = std::static_pointer_cast<ast::type::Str>(result.value());
    //EXPECT(!type->value().compare(U"foo"));
  },

  CASE("List type")
  {
    const auto result = parse(U"Str[]");
    std::shared_ptr<ast::type::List> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::List);
    type = std::static_pointer_cast<ast::type::List>(result.value());
    EXPECT(type->element_type()->kind() == ast::type::Kind::Builtin);
  },

  CASE("Nested list type")
  {
    const auto result = parse(U"Str[][]");
    std::shared_ptr<ast::type::List> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::List);
    type = std::static_pointer_cast<ast::type::List>(result.value());
    EXPECT(type->element_type()->kind() == ast::type::Kind::List);
    type = std::static_pointer_cast<ast::type::List>(type->element_type());
    EXPECT(type->element_type()->kind() == ast::type::Kind::Builtin);
  },

  CASE("Intersection type")
  {
    const auto result = parse(U"Str & Bool");
    std::shared_ptr<ast::type::Intersection> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::Intersection);
    type = std::static_pointer_cast<ast::type::Intersection>(result.value());
    EXPECT(type->types().size() == 2);
  },

  CASE("Union type")
  {
    const auto result = parse(U"Str | Bool");
    std::shared_ptr<ast::type::Union> type;

    EXPECT(result);
    EXPECT(result.value()->kind() == ast::type::Kind::Union);
    type = std::static_pointer_cast<ast::type::Union>(result.value());
    EXPECT(type->types().size() == 2);
  },
};

int
main(int argc, char** argv)
{
  return lest::run(specification, argc, argv);
}
