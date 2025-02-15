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
#include <unordered_set>

#include "snek/interpreter/assign.hpp"
#include "snek/interpreter/error.hpp"
#include "snek/parser/element.hpp"
#include "snek/parser/field.hpp"

namespace snek::interpreter
{
  using callback_type = std::function<void(
    const std::u32string&,
    const value::ptr&
  )>;

  static void
  Process(
    const Runtime&,
    const parser::expression::ptr&,
    const value::ptr&,
    const callback_type&
  );

  static void
  ProcessList(
    const Runtime& runtime,
    const parser::expression::List* variable,
    const value::ptr& value,
    const callback_type& callback
  )
  {
    const auto size = variable->elements.size();
    value::List::size_type list_size;
    const value::List* list;

    if (!value::IsList(value))
    {
      throw runtime.MakeError(
        U"Cannot assign " +
        value::ToString(value::KindOf(value)) +
        U" into " +
        variable->ToString() +
        U"."
      );
    }
    list = static_cast<const value::List*>(value.get());
    list_size = list->GetSize();
    if (list_size < size)
    {
      throw runtime.MakeError(U"List has too few elements for assignment.");
    }
    for (std::size_t i = 0; i < size; ++i)
    {
      const auto& element = variable->elements[i];

      if (element->kind == parser::element::Kind::Value)
      {
        Process(runtime, element->expression, list->At(i), callback);
      } else {
        std::vector<value::ptr> result;

        result.reserve(list_size - i);
        if (i + 1 < size)
        {
          throw runtime.MakeError(U"Variable after `...' variable.");
        }
        do
        {
          result.push_back(list->At(i++));
        }
        while (i < list_size);
        Process(
          runtime,
          element->expression,
          value::List::Make(result),
          callback
        );
      }
    }
  }

  static void
  ProcessRecord(
    const Runtime& runtime,
    const parser::expression::Record* variable,
    const value::ptr& value,
    const callback_type& callback
  )
  {
    const auto size = variable->fields.size();
    std::unordered_set<std::u32string> used_keys;

    if (!value::IsRecord(value))
    {
      throw runtime.MakeError(
        U"Cannot assign " +
        value::ToString(value::KindOf(value)) +
        U" into " +
        variable->ToString() +
        U"."
      );
    }
    for (std::size_t i = 0; i < size; ++i)
    {
      const auto& field = variable->fields[i];
      const auto kind = field->kind();

      if (kind == parser::field::Kind::Named)
      {
        const auto named = static_cast<const parser::field::Named*>(
          field.get()
        );
        const auto& name = named->name;
        const auto property = value::GetProperty(runtime, value, name);

        if (!property)
        {
          throw runtime.MakeError(
            value::ToString(value::KindOf(value)) +
            U" has no property `" +
            name +
            U"'."
          );
        }
        Process(runtime, named->value, *property, callback);
        used_keys.insert(name);
      }
      else if (kind == parser::field::Kind::Shorthand)
      {
        const auto& name = static_cast<const parser::field::Shorthand*>(
          field.get()
        )->name;
        const auto property = value::GetProperty(runtime, value, name);

        if (!property)
        {
          throw runtime.MakeError(
            value::ToString(value::KindOf(value)) +
            U" has no property `" +
            name +
            U"'."
          );
        }
        callback(name, *property);
        used_keys.insert(name);
      }
      else if (kind == parser::field::Kind::Spread)
      {
        std::unordered_map<std::u32string, value::ptr> result;
        const value::Record* r;

        if (i + 1 < size)
        {
          throw runtime.MakeError(U"Variable after `...' variable.");
        }
        r = static_cast<const value::Record*>(value.get());
        for (const auto& f : r->GetOwnPropertyNames())
        {
          if (used_keys.find(f) != std::end(used_keys))
          {
            continue;
          }
          result[f] = *r->GetOwnProperty(f);
        }
        Process(
          runtime,
          static_cast<const parser::field::Spread*>(field.get())->expression,
          value::Record::Make(result),
          callback
        );
      } else {
        throw runtime.MakeError(
          U"Cannot assign to " +
          variable->fields[i]->ToString() +
          U"."
        );
      }
    }
  }

  static void
  Process(
    const Runtime& runtime,
    const parser::expression::ptr& variable,
    const value::ptr& value,
    const callback_type& callback
  )
  {
    if (!variable)
    {
      return;
    }

    switch (variable->kind())
    {
      case parser::expression::Kind::Id:
        callback(
          static_cast<const parser::expression::Id*>(
            variable.get()
          )->identifier,
          value
        );
        break;

      case parser::expression::Kind::List:
        ProcessList(
          runtime,
          static_cast<const parser::expression::List*>(variable.get()),
          value,
          callback
        );
        return;

      case parser::expression::Kind::Record:
        ProcessRecord(
          runtime,
          static_cast<const parser::expression::Record*>(variable.get()),
          value,
          callback
        );
        return;

      default:
        throw runtime.MakeError(
          U"Cannot assign to " +
          variable->ToString() +
          U"."
        );
    }
  }

  void
  AssignTo(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::expression::ptr& variable,
    const value::ptr& value
  )
  {
    Process(
      runtime,
      variable,
      value,
      [&](const std::u32string& name, const value::ptr& value)
      {
        scope->SetVariable(name, value);
      }
    );
  }

  void
  DeclareVar(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::expression::ptr& variable,
    const value::ptr& value,
    bool read_only,
    bool exported
  )
  {
    Process(
      runtime,
      variable,
      value,
      [&](const std::u32string& name, const value::ptr& value)
      {
        scope->DeclareVariable(name, value, read_only, exported);
      }
    );
  }
}
