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
#include "snek/interpreter/resolve.hpp"

namespace snek::interpreter
{
  using namespace snek::parser::statement;

  template<class T>
  inline const T*
  As(const ptr& statement)
  {
    return static_cast<const T*>(statement.get());
  }

  static void
  FindReturnValues(
    const ptr& statement,
    std::vector<parser::expression::ptr>& values
  );

  static void
  FindReturnValuesFromBlock(
    const Block* statement,
    std::vector<parser::expression::ptr>& values
  )
  {
    for (const auto& child : statement->statements())
    {
      FindReturnValues(child, values);
    }
  }

  static void
  FindReturnValuesFromIf(
    const If* statement,
    std::vector<parser::expression::ptr>& values
  )
  {
    FindReturnValues(statement->then_statement(), values);
    FindReturnValues(statement->else_statement(), values);
  }

  static void
  FindReturnValuesFromJump(
    const Jump* statement,
    std::vector<parser::expression::ptr>& values
  )
  {
    if (statement->jump_kind() != JumpKind::Return)
    {
      return;
    }
    if (const auto value = statement->value())
    {
      values.push_back(value);
    }
  }

  static void
  FindReturnValues(
    const ptr& statement,
    std::vector<parser::expression::ptr>& values
  )
  {
    if (!statement)
    {
      return;
    }

    switch (statement->kind())
    {
      case Kind::Block:
        FindReturnValuesFromBlock(As<Block>(statement), values);
        break;

      case Kind::If:
        FindReturnValuesFromIf(As<If>(statement), values);
        break;

      case Kind::Jump:
        FindReturnValuesFromJump(As<Jump>(statement), values);
        break;

      case Kind::While:
        FindReturnValues(As<While>(statement)->body(), values);
        break;

      default:
        break;
    }
  }

  static type::ptr
  ResolveFromReturnStatements(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const ptr& statement
  )
  {
    std::vector<parser::expression::ptr> return_values;
    std::vector<type::ptr> types;

    FindReturnValues(statement, return_values);
    for (const auto& value : return_values)
    {
      types.push_back(
        value ? ResolveExpression(runtime, scope, value) : runtime.void_type()
      );
    }

    return type::Reify(runtime, types);
  }

  static type::ptr
  ResolveDeclareVar(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const DeclareVar* statement
  )
  {
    if (const auto value = statement->value())
    {
      return ResolveExpression(runtime, scope, value);
    }

    return nullptr;
  }

  type::ptr
  ResolveStatement(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const ptr& statement
  )
  {
    if (!statement)
    {
      return nullptr;
    }

    switch (statement->kind())
    {
      case Kind::Block:
      case Kind::If:
      case Kind::Jump:
      case Kind::While:
        return ResolveFromReturnStatements(runtime, scope, statement);

      case Kind::DeclareType:
        return nullptr;

      case Kind::DeclareVar:
        return ResolveDeclareVar(runtime, scope, As<DeclareVar>(statement));

      case Kind::Expression:
        return ResolveExpression(
          runtime,
          scope,
          As<Expression>(statement)->expression()
        );
    }

    return nullptr;
  }
}
