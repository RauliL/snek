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
  using namespace snek::parser::expression;

  template<class T>
  inline const T*
  As(const ptr& expression)
  {
    return static_cast<const T*>(expression.get());
  }

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
    const Assign* expression
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
    const Binary* expression
  )
  {
    switch (expression->op)
    {
      case Binary::Operator::LogicalAnd:
      case Binary::Operator::LogicalOr:
        return type::Reify(
          runtime,
          {
            ResolveExpression(runtime, scope, expression->left),
            ResolveExpression(runtime, scope, expression->right),
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
    const Function* expression
  )
  {
    type::ptr return_type;

    if (expression->return_type)
    {
      return_type = ResolveType(runtime, scope, expression->return_type);
    } else {
      return_type = ResolveStatement(runtime, scope, expression->body);
    }

    return std::make_shared<type::Function>(
      ResolveParameterList(runtime, scope, expression->parameters),
      return_type
    );
  }

  static bool
  ResolveElement(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::element::ptr& element,
    std::vector<type::ptr>& resolved_elements
  )
  {
    const auto type = ResolveExpression(runtime, scope, element->expression);

    if (!type)
    {
      return false;
    }
    else if (element->kind == parser::element::Kind::Spread)
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
  ResolveList(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const List* expression
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
    const Property* expression
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
    const Record* expression
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
    const Ternary* expression
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
    const Unary* expression
  )
  {
    return expression->op == Unary::Operator::Not
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

      // TODO: Maybe return number instead.
      case Kind::Decrement:
        return nullptr;

      case Kind::Float:
        return runtime.float_type();

      case Kind::Function:
        return ResolveFunction(runtime, scope, As<Function>(expression));

      case Kind::Id:
        return nullptr;

      // TODO: Maybe return number instead.
      case Kind::Increment:
        return nullptr;

      case Kind::Int:
        return runtime.int_type();

      case Kind::List:
        return ResolveList(runtime, scope, As<List>(expression));

      case Kind::Null:
        return runtime.void_type();

      case Kind::Property:
        return ResolveProperty(runtime, scope, As<Property>(expression));

      case Kind::Record:
        return ResolveRecord(runtime, scope, As<Record>(expression));

      case Kind::Spread:
        return nullptr;

      case Kind::String:
        return std::make_shared<type::String>(As<String>(expression)->value);

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
