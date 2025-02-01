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
  using namespace snek::parser::expression;

  template<class T>
  inline const T*
  As(const ptr& expression)
  {
    return static_cast<const T*>(expression.get());
  }

  static type::ptr
  ResolveAssign(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const Assign* expression
  )
  {
    return expression->op()
      ? nullptr
      : ResolveExpression(runtime, scope, expression->value());
  }

  static type::ptr
  ResolveBinary(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const Binary* expression
  )
  {
    switch (expression->op())
    {
      case parser::Token::Kind::LogicalAnd:
      case parser::Token::Kind::LogicalOr:
        return type::Reify(
          runtime,
          {
            ResolveExpression(runtime, scope, expression->left()),
            ResolveExpression(runtime, scope, expression->right()),
            runtime.boolean_type(),
            runtime.void_type()
          }
        );

      default:
        return nullptr;
    }
  }

  static type::ptr
  ResolveCall(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const Call* expression
  )
  {
    const auto type = ResolveExpression(
      runtime,
      scope,
      expression->expression()
    );

    if (type && type->kind() == type::Kind::Function)
    {
      const auto function = static_cast<const type::Function*>(type.get());

      if (const auto return_type = function->return_type())
      {
        if (expression->conditional())
        {
          return std::make_shared<type::Union>(
            std::vector<type::ptr>{
              return_type,
              runtime.void_type()
            }
          );
        }

        return return_type;
      }

      return runtime.any_type();
    }

    return nullptr;
  }

  static type::ptr
  ResolveFunction(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const Function* expression
  )
  {
    type::ptr return_type;

    if (const auto unresolved_return_type = expression->return_type())
    {
      return_type = ResolveType(runtime, scope, unresolved_return_type);
    } else {
      return_type = ResolveStatement(runtime, scope, expression->body());
    }

    return std::make_shared<type::Function>(
      ResolveParameterList(runtime, scope, expression->parameters()),
      return_type
    );
  }

  static type::ptr
  ResolveProperty(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const Property* expression
  )
  {
    const auto type = ResolveExpression(
      runtime,
      scope,
      expression->expression()
    );

    if (type && type->kind() == type::Kind::Record)
    {
      const auto& fields = static_cast<const type::Record*>(
        type.get()
      )->fields();
      const auto it = fields.find(expression->name());

      if (it != std::end(fields))
      {
        return expression->conditional()
          ? std::make_shared<type::Union>(std::vector<type::ptr>{
            it->second,
            runtime.void_type()
          })
          : it->second;
      }
    }

    return nullptr;
  }

  static type::ptr
  ResolveTernary(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const Ternary* expression
  )
  {
    return type::Reify(
      runtime,
      {
        ResolveExpression(runtime, scope, expression->then_expression()),
        ResolveExpression(runtime, scope, expression->else_expression()),
      }
    );
  }

  static type::ptr
  ResolveUnary(
    const Runtime& runtime,
    const Scope::ptr&,
    const Unary* expression
  )
  {
    return expression->op() == parser::Token::Kind::Not
      ? runtime.boolean_type()
      : nullptr;
  }

  type::ptr
  ResolveExpression(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const ptr& expression
  )
  {
    if (!expression)
    {
      return nullptr;
    }

    switch (expression->kind())
    {
      case Kind::Assign:
        return ResolveAssign(runtime, scope, As<Assign>(expression));

      case Kind::Binary:
        return ResolveBinary(runtime, scope, As<Binary>(expression));

      case Kind::Boolean:
        return runtime.boolean_type();

      case Kind::Call:
        return ResolveCall(runtime, scope, As<Call>(expression));

      case Kind::Float:
        return runtime.float_type();

      case Kind::Function:
        return ResolveFunction(runtime, scope, As<Function>(expression));

      case Kind::Id:
        return nullptr;

      case Kind::Int:
        return runtime.int_type();

      // TODO: Actually to try resolve the elements contained in the literal.
      case Kind::List:
        return runtime.list_type();

      case Kind::Null:
        return runtime.void_type();

      case Kind::Property:
        return ResolveProperty(runtime, scope, As<Property>(expression));

      // TODO: Actually try to resolve the fields contained in the literal.
      case Kind::Record:
        return runtime.record_type();

      case Kind::String:
        return runtime.string_type();

      // TODO: Add special case for lists and records where an element/field
      // lookup is done.
      case Kind::Subscript:
        return nullptr;

      case Kind::Ternary:
        return ResolveTernary(runtime, scope, As<Ternary>(expression));

      case Kind::Unary:
        return ResolveUnary(runtime, scope, As<Unary>(expression));
    }

    return nullptr;
  }
}
