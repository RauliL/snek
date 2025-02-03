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
#include "snek/interpreter/evaluate.hpp"
#include "snek/interpreter/resolve.hpp"
#include "snek/parser/element.hpp"
#include "snek/parser/field.hpp"

namespace snek::interpreter
{
  using namespace parser::expression;

  template<class T>
  static inline const T*
  As(const ptr& expression)
  {
    return static_cast<const T*>(expression.get());
  }

  template<class T>
  static inline const T*
  As(const parser::field::ptr& field)
  {
    return static_cast<const T*>(field.get());
  }

  template<class T>
  static inline const T*
  As(const value::ptr& value)
  {
    return static_cast<const T*>(value.get());
  }

  static void
  EvaluateElement(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::element::ptr& element,
    std::vector<value::ptr>& list
  )
  {
    switch (element->kind())
    {
      case parser::element::Kind::Spread:
      {
        const auto value = EvaluateExpression(
          runtime,
          scope,
          element->expression()
        );
        const value::List* value_list;
        std::size_t size;

        if (!value::IsList(value))
        {
          throw Error{
            element->position(),
            U"Spread element must be a list."
          };
        }
        value_list = As<value::List>(value);
        size = value_list->GetSize();
        list.reserve(list.size() + size);
        for (std::size_t i = 0; i < size; ++i)
        {
          list.push_back(value_list->At(i));
        }
        break;
      }

      case parser::element::Kind::Value:
        list.push_back(EvaluateExpression(
          runtime,
          scope,
          element->expression()
        ));
        break;
    }
  }

  static void
  EvaluateComputedField(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::field::Computed* field,
    value::Record::container_type& record
  )
  {
    record[
      value::ToString(EvaluateExpression(runtime, scope, field->key()))
    ] = EvaluateExpression(runtime, scope, field->value());
  }

  static void
  EvaluateNamedField(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::field::Named* field,
    value::Record::container_type& record
  )
  {
    record[field->name()] = EvaluateExpression(
      runtime,
      scope,
      field->value()
    );
  }

  static void
  EvaluateShorthandField(
    const Scope::ptr& scope,
    const parser::field::Shorthand* field,
    value::Record::container_type& record
  )
  {
    const auto& name = field->name();

    if (scope)
    {
      value::ptr value;

      if (scope->FindVariable(name, value))
      {
        record[name] = value;
        return;
      }
    }

    throw Error{ field->position(), U"Unknown variable: `" + name + U"'." };
  }

  static void
  EvaluateSpreadField(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::field::Spread* field,
    value::Record::container_type& record
  )
  {
    const auto value = EvaluateExpression(runtime, scope, field->expression());

    if (value::KindOf(value) != value::Kind::Record)
    {
      throw Error{
        field->position(),
        U"Spread element must be a record."
      };
    }
    for (const auto& value_field : *static_cast<const value::Record*>(value.get()))
    {
      record[value_field.first] = value_field.second;
    }
  }

  static void
  EvaluateField(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::field::ptr& field,
    value::Record::container_type& record
  )
  {
    switch (field->kind())
    {
      case parser::field::Kind::Computed:
        EvaluateComputedField(
          runtime,
          scope,
          As<parser::field::Computed>(field),
          record
        );
        break;

      case parser::field::Kind::Named:
        EvaluateNamedField(
          runtime,
          scope,
          As<parser::field::Named>(field),
          record
        );
        break;

      case parser::field::Kind::Shorthand:
        EvaluateShorthandField(
          scope,
          As<parser::field::Shorthand>(field),
          record
        );
        break;

      case parser::field::Kind::Spread:
        EvaluateSpreadField(
          runtime,
          scope,
          As<parser::field::Spread>(field),
          record
        );
        break;
    }
  }

  static value::ptr
  EvaluateAssign(
    Runtime& runtime,
    const Scope::ptr& scope,
    const Assign* expression
  )
  {
    const auto& position = expression->position();
    const auto& op = expression->op();
    const auto& variable = expression->variable();
    auto value = EvaluateExpression(runtime, scope, expression->value());

    if (op)
    {
      value = value::CallMethod(
        runtime,
        EvaluateExpression(runtime, scope, variable),
        Assign::ToString(*op),
        { value },
        position
      );
    }
    if (variable->kind() == Kind::Id)
    {
      if (scope)
      {
        scope->SetVariable(position, As<Id>(variable)->identifier(), value);
      }
    } else {
      throw Error({
        expression->position(),
        U"Cannot assign into `" + variable->ToString() + U"'."
      });
    }

    return value;
  }

  static value::ptr
  EvaluateBinary(
    Runtime& runtime,
    const Scope::ptr& scope,
    const Binary* expression
  )
  {
    const auto left = EvaluateExpression(runtime, scope, expression->left());
    const auto& op = expression->op();

    switch (op)
    {
      case Binary::Operator::LogicalAnd:
        return value::ToBoolean(left)
          ? EvaluateExpression(runtime, scope, expression->right())
          : left;

      case Binary::Operator::LogicalOr:
        return value::ToBoolean(left)
          ? left
          : EvaluateExpression(runtime, scope, expression->right());

      default:
        return value::CallMethod(
          runtime,
          left,
          Binary::ToString(op),
          { EvaluateExpression(runtime, scope, expression->right()) },
          expression->position()
        );
    }
  }

  static void
  EvaluateArgument(
    Runtime& runtime,
    const Scope::ptr& scope,
    const ptr& expression,
    std::vector<value::ptr>& arguments
  )
  {
    if (!expression)
    {
      arguments.push_back(nullptr);
    }
    else if (expression->kind() == Kind::Spread)
    {
      const auto value = EvaluateExpression(
        runtime,
        scope,
        As<Spread>(expression)->expression()
      );

      if (value::IsList(value))
      {
        const auto list = As<value::List>(value);
        const auto size = list->GetSize();

        for (std::size_t i = 0; i < size; ++i)
        {
          arguments.push_back(list->At(i));
        }
      } else {
        throw Error{
          expression->position(),
          U"Cannot spread " + value::ToString(value::KindOf(value)) + U"."
        };
      }
    } else {
      arguments.push_back(EvaluateExpression(runtime, scope, expression));
    }
  }

  static value::ptr
  EvaluateCall(
    Runtime& runtime,
    const Scope::ptr& scope,
    const Call* expression
  )
  {
    const auto value = EvaluateExpression(
      runtime,
      scope,
      expression->expression()
    );

    if (!value && expression->conditional())
    {
      return value;
    }
    else if (value::KindOf(value) == value::Kind::Function)
    {
      const auto function = std::static_pointer_cast<value::Function>(value);
      std::vector<value::ptr> arguments;

      arguments.reserve(expression->arguments().size());
      for (const auto& argument : expression->arguments())
      {
        EvaluateArgument(runtime, scope, argument, arguments);
      }

      return function->Call(expression->position(), runtime, arguments);
    }

    throw Error{
      expression->position(),
      value::ToString(value::KindOf(value)) + U" is not callable."
    };
  }

  static value::ptr
  EvaluateFunction(
    Runtime& runtime,
    const Scope::ptr& scope,
    const Function* expression
  )
  {
    const auto& body = expression->body();
    type::ptr return_type;

    if (const auto unresolved_return_type = expression->return_type())
    {
      return_type = ResolveType(runtime, scope, unresolved_return_type);
    } else {
      return_type = ResolveStatement(runtime, scope, body);
    }

    return value::Function::MakeScripted(
      ResolveParameterList(runtime, scope, expression->parameters()),
      return_type,
      body,
      scope
    );
  }

  static value::ptr
  EvaluateId(const Scope::ptr& scope, const Id* expression)
  {
    const auto& id = expression->identifier();

    if (scope)
    {
      value::ptr slot;

      if (scope->FindVariable(id, slot))
      {
        return slot;
      }
    }

    throw Error{ expression->position(), U"Unknown variable: `" + id + U"'." };
  }

  static value::ptr
  EvaluateList(
    Runtime& runtime,
    const Scope::ptr& scope,
    const List* expression
  )
  {
    const auto& elements = expression->elements();
    const auto size = elements.size();
    std::vector<value::ptr> list;

    list.reserve(size);
    for (std::size_t i = 0; i < size; ++i)
    {
      EvaluateElement(runtime, scope, elements[i], list);
    }

    return value::List::Make(list);
  }

  static value::ptr
  EvaluateProperty(
    Runtime& runtime,
    const Scope::ptr& scope,
    const Property* expression
  )
  {
    const auto value = EvaluateExpression(
      runtime,
      scope,
      expression->expression()
    );

    if (!value && expression->conditional())
    {
      return value;
    }

    if (const auto property = value::GetProperty(
      runtime,
      value,
      expression->name()
    ))
    {
      return *property;
    }

    throw Error{
      expression->position(),
      value::ToString(value::KindOf(value)) +
      U" has no property `" +
      expression->name() +
      U"'."
    };
  }

  static value::ptr
  EvaluateRecord(
    Runtime& runtime,
    const Scope::ptr& scope,
    const Record* expression
  )
  {
    value::Record::container_type record;

    for (const auto& field : expression->fields())
    {
      EvaluateField(runtime, scope, field, record);
    }

    return std::make_shared<value::Record>(record);
  }

  static value::ptr
  EvaluateSubscript(
    Runtime& runtime,
    const Scope::ptr& scope,
    const Subscript* expression
  )
  {
    const auto value = EvaluateExpression(
      runtime,
      scope,
      expression->expression()
    );

    if (!value && expression->conditional())
    {
      return value;
    }

    return value::CallMethod(
      runtime,
      value,
      U"[]",
      { EvaluateExpression(runtime, scope, expression->index()) },
      expression->position()
    );
  }

  static inline value::ptr
  EvaluateTernary(
    Runtime& runtime,
    const Scope::ptr& scope,
    const Ternary* expression
  )
  {
    return EvaluateExpression(
      runtime,
      scope,
      value::ToBoolean(EvaluateExpression(
        runtime,
        scope,
        expression->condition()
      ))
        ? expression->then_expression()
        : expression->else_expression()
    );
  }

  static std::u32string
  GetMethodName(Unary::Operator op)
  {
    switch (op)
    {
      case Unary::Operator::Add:
        return U"+@";

      case Unary::Operator::BitwiseNot:
        return U"~";

      case Unary::Operator::Not:
        return U"!";

      case Unary::Operator::Sub:
        return U"-@";
    }

    throw Error{ std::nullopt, U"Unknown unary operator." };
  }

  static value::ptr
  EvaluateUnary(
    Runtime& runtime,
    const Scope::ptr& scope,
    const Unary* expression
  )
  {
    const auto op = expression->op();
    const auto operand = EvaluateExpression(
      runtime,
      scope,
      expression->operand()
    );

    if (op == Unary::Operator::Not)
    {
      return std::make_shared<value::Boolean>(!value::ToBoolean(operand));
    }

    return value::CallMethod(
      runtime,
      operand,
      GetMethodName(op),
      {},
      expression->position()
    );
  }

  value::ptr
  EvaluateExpression(
    Runtime& runtime,
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
        return EvaluateAssign(runtime, scope, As<Assign>(expression));

      case Kind::Binary:
        return EvaluateBinary(runtime, scope, As<Binary>(expression));

      case Kind::Boolean:
        return std::make_shared<value::Boolean>(
          As<Boolean>(expression)->value()
        );

      case Kind::Call:
        return EvaluateCall(runtime, scope, As<Call>(expression));

      case Kind::Float:
        return std::make_shared<value::Float>(
          As<Float>(expression)->value()
        );

      case Kind::Function:
        return EvaluateFunction(runtime, scope, As<Function>(expression));

      case Kind::Id:
        return EvaluateId(scope, As<Id>(expression));

      case Kind::Int:
        return std::make_shared<value::Int>(
          As<Int>(expression)->value()
        );

      case Kind::List:
        return EvaluateList(runtime, scope, As<List>(expression));

      case Kind::Null:
        return nullptr;

      case Kind::Property:
        return EvaluateProperty(runtime, scope, As<Property>(expression));

      case Kind::Record:
        return EvaluateRecord(runtime, scope, As<Record>(expression));

      case Kind::Spread:
        throw Error{
          expression->position(),
          U"Unexpected spread expression."
        };

      case Kind::String:
        return std::make_shared<value::String>(
          As<String>(expression)->value()
        );

      case Kind::Subscript:
        return EvaluateSubscript(runtime, scope, As<Subscript>(expression));

      case Kind::Ternary:
        return EvaluateTernary(runtime, scope, As<Ternary>(expression));

      case Kind::Unary:
        return EvaluateUnary(runtime, scope, As<Unary>(expression));
    }

    return nullptr;
  }
}
