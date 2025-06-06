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
#include "snek/interpreter/assign.hpp"
#include "snek/interpreter/error.hpp"
#include "snek/interpreter/evaluate.hpp"
#include "snek/interpreter/execute.hpp"
#include "snek/interpreter/jump.hpp"
#include "snek/interpreter/resolve.hpp"
#include "snek/parser/import.hpp"

namespace snek::interpreter
{
  using namespace parser::statement;

  template<class T>
  static inline const T*
  As(const ptr& statement)
  {
    return static_cast<const T*>(statement.get());
  }

  static value::ptr
  ExecuteBlock(
    Runtime& runtime,
    const Scope::ptr& scope,
    const Block* statement
  )
  {
    const auto size = statement->statements.size();

    for (std::size_t i = 0; i < size; ++i)
    {
      ExecuteStatement(runtime, scope, statement->statements[i]);
    }

    return nullptr;
  }

  static value::ptr
  ExecuteDeclareType(
    Runtime& runtime,
    const Scope::ptr& scope,
    const DeclareType* statement
  )
  {
    scope->DeclareType(
      statement->name,
      ResolveType(runtime, scope, statement->type),
      statement->is_export
    );

    return nullptr;
  }

  static value::ptr
  ExecuteDeclareVar(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::statement::DeclareVar* statement
  )
  {
    const auto value = EvaluateExpression(runtime, scope, statement->value);

    DeclareVar(
      runtime,
      scope,
      statement->variable,
      value,
      statement->is_read_only,
      statement->is_export
    );

    return value;
  }

  static value::ptr
  ExecuteIf(
    Runtime& runtime,
    const Scope::ptr& scope,
    const If* statement
  )
  {
    const auto condition = value::ToBoolean(
      EvaluateExpression(runtime, scope, statement->condition)
    );

    if (condition)
    {
      return ExecuteStatement(runtime, scope, statement->then_statement);
    }
    else if (statement->else_statement)
    {
      return ExecuteStatement(runtime, scope, statement->else_statement);
    }

    return nullptr;
  }

  static void
  ImportNamed(
    const Runtime& runtime,
    const Scope::ptr& module,
    const Scope::ptr& scope,
    const parser::import::Named* specifier
  )
  {
    value::ptr value_slot;

    if (module->FindVariable(specifier->name, value_slot, true))
    {
      scope->DeclareVariable(
        specifier->alias ? *specifier->alias : specifier->name,
        value_slot,
        true,
        false
      );
      return;
    } else {
      type::ptr type_slot;

      if (module->FindType(specifier->name, type_slot, true))
      {
        scope->DeclareType(
          specifier->alias ? *specifier->alias : specifier->name,
          type_slot,
          false
        );
        return;
      }
    }

    throw runtime.MakeError(
      U"Module does not export `" +
      specifier->name +
      U"'."
    );
  }

  static void
  ImportStar(
    const Scope::ptr& module,
    const Scope::ptr& scope,
    const parser::import::Star* specifier
  )
  {
    const auto exported_variables = module->GetExportedVariables();

    if (specifier->alias)
    {
      std::unordered_map<std::u32string, value::ptr> fields;

      // TODO: Find out what to do with exported types.
      for (const auto& variable : exported_variables)
      {
        fields[variable.first] = variable.second;
      }
      scope->DeclareVariable(
        *specifier->alias,
        value::Record::Make(fields),
        true,
        false
      );
    } else {
      for (const auto& variable : exported_variables)
      {
        scope->DeclareVariable(
          variable.first,
          variable.second,
          true,
          false
        );
      }
      for (const auto& type : module->GetExportedTypes())
      {
        scope->DeclareType(type.first, type.second, false);
      }
    }
  }

  static value::ptr
  ExecuteImport(
    Runtime& runtime,
    const Scope::ptr& scope,
    const Import* statement
  )
  {
    const auto module = runtime.ImportModule(statement->path);

    for (const auto& specifier : statement->specifiers)
    {
      if (!specifier)
      {
        continue;
      }
      switch (specifier->kind())
      {
        case parser::import::Kind::Named:
          ImportNamed(
            runtime,
            module,
            scope,
            static_cast<const parser::import::Named*>(specifier.get())
          );
          break;

        case parser::import::Kind::Star:
          ImportStar(
            module,
            scope,
            static_cast<const parser::import::Star*>(specifier.get())
          );
          break;
      }
    }

    return nullptr;
  }

  static value::ptr
  ExecuteWhile(
    Runtime& runtime,
    const Scope::ptr& scope,
    const While* statement
  )
  {
    value::ptr value;

    for (;;)
    {
      try
      {
        const auto condition = value::ToBoolean(
          EvaluateExpression(runtime, scope, statement->condition)
        );

        if (!condition)
        {
          break;
        }
        value = ExecuteStatement(runtime, scope, statement->body);
      }
      catch (const Jump& jump)
      {
        if (jump.kind() == JumpKind::Break)
        {
          break;
        }
        else if (jump.kind() == JumpKind::Continue)
        {
          throw jump;
        }
      }
    }

    return value;
  }

  static void
  ExecuteJump(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::statement::Jump* statement
  )
  {
    value::ptr value;

    if (statement->value)
    {
      value = EvaluateExpression(
        runtime,
        scope,
        statement->value,
        statement->jump_kind == JumpKind::Return
      );
    }

    throw Jump(statement->position, statement->jump_kind, value);
  }

  value::ptr
  ExecuteStatement(
    Runtime& runtime,
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
        return ExecuteBlock(runtime, scope, As<Block>(statement));

      case Kind::DeclareType:
        return ExecuteDeclareType(runtime, scope, As<DeclareType>(statement));

      case Kind::DeclareVar:
        return ExecuteDeclareVar(
          runtime,
          scope,
          As<parser::statement::DeclareVar>(statement)
        );

      case Kind::Expression:
        return EvaluateExpression(
          runtime,
          scope,
          As<Expression>(statement)->expression
        );

      case Kind::If:
        return ExecuteIf(runtime, scope, As<If>(statement));

      case Kind::Import:
        return ExecuteImport(runtime, scope, As<Import>(statement));

      case Kind::Jump:
        ExecuteJump(
          runtime,
          scope,
          As<parser::statement::Jump>(statement)
        );
        break;

      case Kind::While:
        return ExecuteWhile(runtime, scope, As<While>(statement));
    }

    return nullptr;
  }
}
