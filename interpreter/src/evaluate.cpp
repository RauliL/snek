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
#include "snek/interpreter/resolve.hpp"
#include "snek/parser/element.hpp"
#include "snek/parser/field.hpp"

namespace snek::interpreter
{
  template<class T>
  static inline const T*
  As(const parser::expression_ptr& expression)
  {
    return static_cast<const T*>(expression.get());
  }

  template<class T>
  static inline const T*
  As(const parser::field_ptr& field)
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
    const parser::element_ptr& element,
    std::vector<value::ptr>& list
  )
  {
    switch (element->kind)
    {
      case parser::ElementKind::Spread:
      {
        const auto value = EvaluateExpression(
          runtime,
          scope,
          element->expression
        );
        const value::List* value_list;
        std::size_t size;

        if (!value::IsList(value))
        {
          throw runtime.MakeError(U"Spread element must be a list.");
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

      case parser::ElementKind::Value:
        list.push_back(EvaluateExpression(
          runtime,
          scope,
          element->expression
        ));
        break;
    }
  }

  static void
  EvaluateComputedField(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Field::Computed* field,
    std::unordered_map<std::u32string, value::ptr>& record
  )
  {
    record[
      value::ToString(EvaluateExpression(runtime, scope, field->key))
    ] = EvaluateExpression(runtime, scope, field->value);
  }

  static void
  EvaluateFunctionField(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Field::Function* field,
    std::unordered_map<std::u32string, value::ptr>& record
  )
  {
    record[field->name] = value::Function::MakeScripted(
      ResolveParameterList(
        runtime,
        scope,
        field->signature.parameters
      ),
      ResolveReturnType(
        runtime,
        scope,
        field->signature.return_type,
        field->body
      ),
      field->body,
      scope
    );
  }

  static void
  EvaluateNamedField(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Field::Named* field,
    std::unordered_map<std::u32string, value::ptr>& record
  )
  {
    record[field->name] = EvaluateExpression(
      runtime,
      scope,
      field->value
    );
  }

  static void
  EvaluateShorthandField(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Field::Shorthand* field,
    std::unordered_map<std::u32string, value::ptr>& record
  )
  {
    if (scope)
    {
      value::ptr value;

      if (scope->FindVariable(field->name, value))
      {
        record[field->name] = value;
        return;
      }
    }

    throw runtime.MakeError(U"Unknown variable: `" + field->name + U"'.");
  }

  static void
  EvaluateSpreadField(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Field::Spread* field,
    std::unordered_map<std::u32string, value::ptr>& record
  )
  {
    const auto value = EvaluateExpression(runtime, scope, field->expression);

    if (value::KindOf(value) != value::Kind::Record)
    {
      throw runtime.MakeError(U"Spread element must be a record.");
    }
    static_cast<const value::Record*>(value.get())->ForEach(
      [&record](const auto& key, const auto& value)
      {
        record[key] = value;
      }
    );
  }

  static void
  EvaluateField(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::field_ptr& field,
    std::unordered_map<std::u32string, value::ptr>& record
  )
  {
    switch (field->kind)
    {
      case parser::FieldKind::Computed:
        EvaluateComputedField(
          runtime,
          scope,
          As<parser::Field::Computed>(field),
          record
        );
        break;

      case parser::FieldKind::Function:
        EvaluateFunctionField(
          runtime,
          scope,
          As<parser::Field::Function>(field),
          record
        );
        break;

      case parser::FieldKind::Named:
        EvaluateNamedField(
          runtime,
          scope,
          As<parser::Field::Named>(field),
          record
        );
        break;

      case parser::FieldKind::Shorthand:
        EvaluateShorthandField(
          runtime,
          scope,
          As<parser::Field::Shorthand>(field),
          record
        );
        break;

      case parser::FieldKind::Spread:
        EvaluateSpreadField(
          runtime,
          scope,
          As<parser::Field::Spread>(field),
          record
        );
        break;
    }
  }

  static value::ptr
  EvaluateAssign(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Expression::Assign* expression
  )
  {
    value::ptr new_value;

    if (expression->op)
    {
      switch (*expression->op)
      {
        case parser::AssignOp::LogicalAnd:
          {
            const auto old_value = EvaluateExpression(
              runtime,
              scope,
              expression->variable
            );

            if (!value::ToBoolean(old_value))
            {
              return old_value;
            }
            new_value = EvaluateExpression(
              runtime,
              scope,
              expression->value
            );
          }
          break;

        case parser::AssignOp::LogicalOr:
          {
            const auto old_value = EvaluateExpression(
              runtime,
              scope,
              expression->variable
            );

            if (value::ToBoolean(old_value))
            {
              return old_value;
            }
            new_value = EvaluateExpression(
              runtime,
              scope,
              expression->value
            );
          }
          break;

        case parser::AssignOp::NullCoalescing:
          {
            const auto old_value = EvaluateExpression(
              runtime,
              scope,
              expression->variable
            );

            if (old_value)
            {
              return old_value;
            }
            new_value = EvaluateExpression(
              runtime,
              scope,
              expression->value
            );
          }
          break;

        default:
          new_value = value::CallMethod(
            runtime,
            EvaluateExpression(runtime, scope, expression->variable),
            parser::ToString(*expression->op),
            { EvaluateExpression(runtime, scope, expression->value) },
            expression->position
          );
          break;
      }
    } else {
      new_value = EvaluateExpression(runtime, scope, expression->value);
    }
    AssignTo(runtime, scope, expression->variable, new_value);

    return new_value;
  }

  static value::ptr
  EvaluateBinary(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Expression::Binary* expression,
    bool tail_call
  )
  {
    const auto left = EvaluateExpression(runtime, scope, expression->left);

    switch (expression->op)
    {
      case parser::BinaryOp::LogicalAnd:
        return value::ToBoolean(left)
          ? EvaluateExpression(runtime, scope, expression->right)
          : left;

      case parser::BinaryOp::LogicalOr:
        return value::ToBoolean(left)
          ? left
          : EvaluateExpression(runtime, scope, expression->right);

      case parser::BinaryOp::NullCoalescing:
        return left
          ? left
          : EvaluateExpression(runtime, scope, expression->right);

      default:
        return value::CallMethod(
          runtime,
          left,
          parser::ToString(expression->op),
          { EvaluateExpression(runtime, scope, expression->right) },
          expression->position,
          tail_call
        );
    }
  }

  static void
  EvaluateArgument(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::expression_ptr& expression,
    std::vector<value::ptr>& arguments
  )
  {
    if (!expression)
    {
      arguments.push_back(nullptr);
    }
    else if (expression->Is(parser::ExpressionKind::Spread))
    {
      const auto value = EvaluateExpression(
        runtime,
        scope,
        As<parser::Expression::Spread>(expression)->expression
      );

      if (value::IsList(value))
      {
        As<value::List>(value)->ForEach(
          [&arguments](const auto& value, auto)
          {
            arguments.push_back(value);
          }
        );
      } else {
        throw runtime.MakeError(
          U"Cannot spread " +
          value::ToString(value::KindOf(value)) +
          U"."
        );
      }
    } else {
      arguments.push_back(EvaluateExpression(runtime, scope, expression));
    }
  }

  static value::ptr
  EvaluateCall(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Expression::Call* expression,
    bool tail_call
  )
  {
    const auto value = EvaluateExpression(
      runtime,
      scope,
      expression->expression
    );

    if (!value && expression->conditional)
    {
      return value;
    }
    else if (value::KindOf(value) == value::Kind::Function)
    {
      std::vector<value::ptr> arguments;

      arguments.reserve(expression->arguments.size());
      for (const auto& argument : expression->arguments)
      {
        EvaluateArgument(runtime, scope, argument, arguments);
      }

      return value::Function::Call(
        runtime,
        std::static_pointer_cast<value::Function>(value),
        arguments,
        tail_call,
        expression->position
      );
    }

    throw runtime.MakeError(
      value::ToString(value::KindOf(value)) +
      U" is not callable."
    );
  }

  static value::ptr
  EvaluateDecrement(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Expression::Decrement* expression,
    bool tail_call
  )
  {
    auto value = EvaluateExpression(runtime, scope, expression->variable);
    const auto new_value = value::CallMethod(
      runtime,
      value,
      U"-",
      { runtime.MakeInt(1) },
      expression->position,
      tail_call
    );

    AssignTo(runtime, scope, expression->variable, new_value);

    return expression->pre ? new_value : value;
  }

  static value::ptr
  EvaluateFunction(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Expression::Function* expression
  )
  {
    return value::Function::MakeScripted(
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
      ),
      expression->body,
      scope
    );
  }

  static value::ptr
  EvaluateId(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Expression::Id* expression
  )
  {
    if (scope)
    {
      value::ptr slot;

      if (scope->FindVariable(expression->identifier, slot))
      {
        return slot;
      }
    }

    throw runtime.MakeError(
      U"Unknown variable: `" +
      expression->identifier +
      U"'."
    );
  }

  static value::ptr
  EvaluateIncrement(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Expression::Increment* expression,
    bool tail_call
  )
  {
    auto value = EvaluateExpression(runtime, scope, expression->variable);
    const auto new_value = value::CallMethod(
      runtime,
      value,
      U"+",
      { runtime.MakeInt(1) },
      expression->position,
      tail_call
    );

    AssignTo(runtime, scope, expression->variable, new_value);

    return expression->pre ? new_value : value;
  }

  static value::ptr
  EvaluateList(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Expression::List* expression
  )
  {
    const auto size = expression->elements.size();
    std::vector<value::ptr> list;

    list.reserve(size);
    for (std::size_t i = 0; i < size; ++i)
    {
      EvaluateElement(runtime, scope, expression->elements[i], list);
    }

    return value::List::Make(list);
  }

  static value::ptr
  EvaluateProperty(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Expression::Property* expression
  )
  {
    const auto value = EvaluateExpression(
      runtime,
      scope,
      expression->expression
    );

    if (!value && expression->conditional)
    {
      return value;
    }

    if (const auto property = value::GetProperty(
      runtime,
      value,
      expression->name
    ))
    {
      return *property;
    }

    throw runtime.MakeError(
      value::ToString(value::KindOf(value)) +
      U" has no property `" +
      expression->name +
      U"'."
    );
  }

  static value::ptr
  EvaluateRecord(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Expression::Record* expression
  )
  {
    std::unordered_map<std::u32string, value::ptr> record;

    for (const auto& field : expression->fields)
    {
      EvaluateField(runtime, scope, field, record);
    }

    return value::Record::Make(record);
  }

  static value::ptr
  EvaluateSubscript(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Expression::Subscript* expression,
    bool tail_call
  )
  {
    const auto value = EvaluateExpression(
      runtime,
      scope,
      expression->expression
    );

    if (!value && expression->conditional)
    {
      return value;
    }

    return value::CallMethod(
      runtime,
      value,
      U"[]",
      { EvaluateExpression(runtime, scope, expression->index) },
      expression->position,
      tail_call
    );
  }

  static inline value::ptr
  EvaluateTernary(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Expression::Ternary* expression
  )
  {
    return EvaluateExpression(
      runtime,
      scope,
      value::ToBoolean(EvaluateExpression(
        runtime,
        scope,
        expression->condition
      ))
        ? expression->then_expression
        : expression->else_expression
    );
  }

  static std::u32string
  GetMethodName(parser::UnaryOp op)
  {
    switch (op)
    {
      case parser::UnaryOp::Add:
        return U"+@";

      case parser::UnaryOp::BitwiseNot:
        return U"~";

      case parser::UnaryOp::Not:
        return U"!";

      case parser::UnaryOp::Sub:
        return U"-@";
    }

    throw Error{ {}, U"Unknown unary operator." };
  }

  static value::ptr
  EvaluateUnary(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Expression::Unary* expression,
    bool tail_call
  )
  {
    const auto operand = EvaluateExpression(
      runtime,
      scope,
      expression->operand
    );

    if (expression->op == parser::UnaryOp::Not)
    {
      return runtime.MakeBoolean(!value::ToBoolean(operand));
    }

    return value::CallMethod(
      runtime,
      operand,
      GetMethodName(expression->op),
      {},
      expression->position,
      tail_call
    );
  }

  value::ptr
  EvaluateExpression(
    Runtime& runtime,
    const Scope::ptr& scope,
    const parser::expression_ptr& expression,
    bool tail_call
  )
  {
    if (!expression)
    {
      return nullptr;
    }

    switch (expression->kind)
    {
      case parser::ExpressionKind::Assign:
        return EvaluateAssign(
          runtime,
          scope,
          As<parser::Expression::Assign>(expression)
        );

      case parser::ExpressionKind::Binary:
        return EvaluateBinary(
          runtime,
          scope,
          As<parser::Expression::Binary>(expression),
          tail_call
        );

      case parser::ExpressionKind::Boolean:
        return runtime.MakeBoolean(
          As<parser::Expression::Boolean>(expression)->value
        );

      case parser::ExpressionKind::Call:
        return EvaluateCall(
          runtime,
          scope,
          As<parser::Expression::Call>(expression),
          tail_call
        );

      case parser::ExpressionKind::Decrement:
        return EvaluateDecrement(
          runtime,
          scope,
          As<parser::Expression::Decrement>(expression),
          tail_call
        );

      case parser::ExpressionKind::Float:
        return std::make_shared<value::Float>(
          As<parser::Expression::Float>(expression)->value
        );

      case parser::ExpressionKind::Function:
        return EvaluateFunction(
          runtime,
          scope,
          As<parser::Expression::Function>(expression)
        );

      case parser::ExpressionKind::Id:
        return EvaluateId(
          runtime,
          scope,
          As<parser::Expression::Id>(expression)
        );

      case parser::ExpressionKind::Increment:
        return EvaluateIncrement(
          runtime,
          scope,
          As<parser::Expression::Increment>(expression),
          tail_call
        );

      case parser::ExpressionKind::Int:
        return runtime.MakeInt(As<parser::Expression::Int>(expression)->value);

      case parser::ExpressionKind::List:
        return EvaluateList(
          runtime,
          scope,
          As<parser::Expression::List>(expression)
        );

      case parser::ExpressionKind::Null:
        return nullptr;

      case parser::ExpressionKind::Property:
        return EvaluateProperty(
          runtime,
          scope,
          As<parser::Expression::Property>(expression)
        );

      case parser::ExpressionKind::Record:
        return EvaluateRecord(
          runtime,
          scope,
          As<parser::Expression::Record>(expression)
        );

      case parser::ExpressionKind::Spread:
        throw runtime.MakeError(U"Unexpected spread expression.");

      case parser::ExpressionKind::String:
        return value::String::Make(
          As<parser::Expression::String>(expression)->value
        );

      case parser::ExpressionKind::Subscript:
        return EvaluateSubscript(
          runtime,
          scope,
          As<parser::Expression::Subscript>(expression),
          tail_call
        );

      case parser::ExpressionKind::Ternary:
        return EvaluateTernary(
          runtime,
          scope,
          As<parser::Expression::Ternary>(expression)
        );

      case parser::ExpressionKind::Unary:
        return EvaluateUnary(
          runtime,
          scope,
          As<parser::Expression::Unary>(expression),
          tail_call
        );
    }

    return nullptr;
  }
}
