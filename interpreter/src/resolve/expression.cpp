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
#include "snek/parser/element.hpp"
#include "snek/parser/field.hpp"

namespace snek::interpreter
{
  template<class T>
  inline const T*
  As(const type::ptr& type)
  {
    return static_cast<const T*>(type.get());
  }

  static type::ptr
  ResolveAssign(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Expression::Assign* expression
  )
  {
    return expression->op
      ? nullptr
      : ResolveExpression(runtime, scope, expression->value);
  }

  static type::ptr
  ResolveBinary(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Expression::Binary* expression
  )
  {
    switch (expression->op)
    {
      case parser::BinaryOp::LogicalAnd:
      case parser::BinaryOp::LogicalOr:
        return type::Reify(
          runtime,
          {
            ResolveExpression(runtime, scope, expression->left),
            ResolveExpression(runtime, scope, expression->right),
            runtime.boolean_type(),
            runtime.void_type()
          }
        );

      // TODO: Try to find the operator method, and use it's return type when
      // possible.
      default:
        return nullptr;
    }
  }

  static type::ptr
  ResolveCall(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Expression::Call* expression
  )
  {
    const auto type = ResolveExpression(
      runtime,
      scope,
      expression->expression
    );

    if (type && type->kind() == type::Kind::Function)
    {
      const auto function = static_cast<const type::Function*>(type.get());

      if (const auto return_type = function->return_type())
      {
        if (expression->conditional)
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
    const parser::Expression::Function* expression
  )
  {
    return std::make_shared<type::Function>(
      ResolveParameterList(
        runtime,
        scope,
        expression->signature.parameters
      ),
      ResolveReturnType(
        runtime,
        scope,
        expression->signature.return_type,
        expression->body
      )
    );
  }

  static bool
  ResolveElement(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::element_ptr& element,
    std::vector<type::ptr>& resolved_elements
  )
  {
    const auto type = ResolveExpression(runtime, scope, element->expression);

    if (!type)
    {
      return false;
    }
    else if (element->kind == parser::ElementKind::Spread)
    {
      if (type->kind() == type::Kind::Tuple)
      {
        const auto& types = As<type::Tuple>(type)->types();

        resolved_elements.insert(
          std::end(resolved_elements),
          std::begin(types),
          std::end(types)
        );

        return true;
      }

      return false;
    }
    resolved_elements.push_back(type);

    return true;
  }

  static type::ptr
  ResolveId(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Expression::Id* expression
  )
  {
    value::ptr slot;

    if (scope->FindVariable(expression->identifier, slot, true))
    {
      return ResolveValue(runtime, slot);
    }

    return nullptr;
  }

  static type::ptr
  ResolveList(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Expression::List* expression
  )
  {
    std::vector<type::ptr> resolved_elements;

    for (const auto& element : expression->elements)
    {
      if (!ResolveElement(runtime, scope, element, resolved_elements))
      {
        return runtime.list_type();
      }
    }

    return std::make_shared<type::Tuple>(resolved_elements);
  }

  static type::ptr
  ResolveProperty(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Expression::Property* expression
  )
  {
    const auto type = ResolveExpression(
      runtime,
      scope,
      expression->expression
    );

    if (type && type->kind() == type::Kind::Record)
    {
      const auto& fields = As<type::Record>(type)->fields();
      const auto it = fields.find(expression->name);

      if (it != std::end(fields))
      {
        return expression->conditional
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
  ResolveRecord(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Expression::Record* expression
  )
  {
    type::Record::container_type resolved_fields;

    for (const auto& field : expression->fields)
    {
      if (!ResolveField(runtime, scope, field, resolved_fields))
      {
        return runtime.record_type();
      }
    }

    return std::make_shared<type::Record>(resolved_fields);
  }

  static type::ptr
  ResolveTernary(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Expression::Ternary* expression
  )
  {
    // TODO: Check if condition is constant.
    return type::Reify(
      runtime,
      {
        ResolveExpression(runtime, scope, expression->then_expression),
        ResolveExpression(runtime, scope, expression->else_expression),
      }
    );
  }

  static type::ptr
  ResolveUnary(
    const Runtime& runtime,
    const Scope::ptr&,
    const parser::Expression::Unary* expression
  )
  {
    return expression->op == parser::UnaryOp::Not
      ? runtime.boolean_type()
      : nullptr;
  }

  type::ptr
  ResolveExpression(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::expression_ptr& expression
  )
  {
    if (!expression)
    {
      return nullptr;
    }

    switch (expression->kind)
    {
      case parser::ExpressionKind::Assign:
        return ResolveAssign(
          runtime,
          scope,
          As<parser::Expression::Assign>(expression)
        );

      case parser::ExpressionKind::Binary:
        return ResolveBinary(
          runtime,
          scope,
          As<parser::Expression::Binary>(expression)
        );

      case parser::ExpressionKind::Boolean:
        return runtime.boolean_type();

      case parser::ExpressionKind::Call:
        return ResolveCall(
          runtime,
          scope,
          As<parser::Expression::Call>(expression)
        );

      // TODO: Maybe return number instead.
      case parser::ExpressionKind::Decrement:
        return nullptr;

      case parser::ExpressionKind::Float:
        return runtime.float_type();

      case parser::ExpressionKind::Function:
        return ResolveFunction(
          runtime,
          scope,
          As<parser::Expression::Function>(expression)
        );

      case parser::ExpressionKind::Id:
        return ResolveId(
          runtime,
          scope,
          As<parser::Expression::Id>(expression)
        );

      // TODO: Maybe return number instead.
      case parser::ExpressionKind::Increment:
        return nullptr;

      case parser::ExpressionKind::Int:
        return runtime.int_type();

      case parser::ExpressionKind::List:
        return ResolveList(
          runtime,
          scope,
          As<parser::Expression::List>(expression)
        );

      case parser::ExpressionKind::Null:
        return runtime.void_type();

      case parser::ExpressionKind::Property:
        return ResolveProperty(
          runtime,
          scope,
          As<parser::Expression::Property>(expression)
        );

      case parser::ExpressionKind::Record:
        return ResolveRecord(
          runtime,
          scope,
          As<parser::Expression::Record>(expression)
        );

      case parser::ExpressionKind::Spread:
        return nullptr;

      case parser::ExpressionKind::String:
        return std::make_shared<type::String>(
          As<parser::Expression::String>(expression)->value
        );

      // TODO: Add special case for lists and records where an element/field
      // lookup is done.
      case parser::ExpressionKind::Subscript:
        return nullptr;

      case parser::ExpressionKind::Ternary:
        return ResolveTernary(
          runtime,
          scope,
          As<parser::Expression::Ternary>(expression)
        );

      case parser::ExpressionKind::Unary:
        return ResolveUnary(
          runtime,
          scope,
          As<parser::Expression::Unary>(expression)
        );
    }

    return nullptr;
  }

  type::ptr
  ResolveReturnType(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::type_ptr& return_type,
    const parser::statement_ptr& body
  )
  {
    // TODO: Create new fake scope and populate it with parameters and local
    // variables.
    const auto actual_return_type = ResolveStatement(runtime, scope, body);
    type::ptr resolved_return_type;

    if (!return_type)
    {
      return actual_return_type;
    }
    resolved_return_type = ResolveType(runtime, scope, return_type);
    if (!actual_return_type->Accepts(resolved_return_type))
    {
      throw runtime.MakeError(
        U"Return type "
        + resolved_return_type->ToString()
        + U" does not match actual return type "
        + actual_return_type->ToString()
        + U"."
      );
    }

    return resolved_return_type;
  }
}
