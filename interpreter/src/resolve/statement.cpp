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

#include "./utils.hpp"

namespace snek::interpreter
{
  template<class T>
  inline const T*
  As(const parser::statement_ptr& statement)
  {
    return static_cast<const T*>(statement.get());
  }

  static void
  FindReturnValues(
    const parser::statement_ptr& statement,
    std::vector<parser::expression_ptr>& values
  );

  static void
  FindReturnValuesFromBlock(
    const parser::Statement::Block* statement,
    std::vector<parser::expression_ptr>& values
  )
  {
    const auto size = statement->statements.size();

    for (std::size_t i = 0; i < size; ++i)
    {
      FindReturnValues(statement->statements[i], values);
    }
  }

  static void
  FindReturnValuesFromIf(
    const parser::Statement::If* statement,
    std::vector<parser::expression_ptr>& values
  )
  {
    FindReturnValues(statement->then_statement, values);
    FindReturnValues(statement->else_statement, values);
  }

  static void
  FindReturnValuesFromJump(
    const parser::Statement::Jump* statement,
    std::vector<parser::expression_ptr>& values
  )
  {
    if (statement->jump_kind != parser::JumpKind::Return)
    {
      return;
    }
    values.push_back(statement->value ? statement->value : nullptr);
  }

  static void
  FindReturnValues(
    const parser::statement_ptr& statement,
    std::vector<parser::expression_ptr>& values
  )
  {
    if (!statement)
    {
      return;
    }

    switch (statement->kind)
    {
      case parser::StatementKind::Block:
        FindReturnValuesFromBlock(
          As<parser::Statement::Block>(statement),
          values
        );
        break;

      case parser::StatementKind::If:
        FindReturnValuesFromIf(
          As<parser::Statement::If>(statement),
          values
        );
        break;

      case parser::StatementKind::Jump:
        FindReturnValuesFromJump(
          As<parser::Statement::Jump>(statement),
          values
        );
        break;

      case parser::StatementKind::While:
        FindReturnValues(
          As<parser::Statement::While>(statement)->body,
          values
        );
        break;

      default:
        break;
    }
  }

  static type::ptr
  ResolveFromReturnStatements(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::statement_ptr& statement
  )
  {
    std::vector<parser::expression_ptr> return_values;
    std::vector<type::ptr> types;

    FindReturnValues(statement, return_values);
    for (const auto& value : return_values)
    {
      types.push_back(
        value
          ? utils::TypeOrAny(runtime, ResolveExpression(runtime, scope, value))
          : runtime.void_type()
      );
    }

    return type::Reify(runtime, types);
  }

  static type::ptr
  ResolveDeclareVar(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Statement::DeclareVar* statement
  )
  {
    if (statement->value)
    {
      return ResolveExpression(runtime, scope, statement->value);
    }

    return nullptr;
  }

  type::ptr
  ResolveStatement(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::statement_ptr& statement
  )
  {
    if (!statement)
    {
      return nullptr;
    }

    switch (statement->kind)
    {
      case parser::StatementKind::Block:
      case parser::StatementKind::If:
      case parser::StatementKind::Jump:
      case parser::StatementKind::While:
        return ResolveFromReturnStatements(runtime, scope, statement);

      case parser::StatementKind::DeclareType:
        return nullptr;

      case parser::StatementKind::DeclareVar:
        return ResolveDeclareVar(
          runtime,
          scope,
          As<parser::Statement::DeclareVar>(statement)
        );

      case parser::StatementKind::Expression:
        return ResolveExpression(
          runtime,
          scope,
          As<parser::Statement::Expression>(statement)->expression
        );

      case parser::StatementKind::Import:
        return nullptr;
    }

    return nullptr;
  }
}
