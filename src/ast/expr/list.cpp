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
#include <snek/ast/expr/list.hpp>
#include <snek/type/base.hpp>
#include <snek/value/list.hpp>

namespace snek::ast::expr
{
  List::List(
    const Position& position,
    const container_type& elements
  )
    : LValue(position)
    , m_elements(elements) {}

  std::u32string
  List::to_string() const
  {
    std::u32string result;

    result += U'[';
    for (std::size_t i = 0; i < m_elements.size(); ++i)
    {
      if (i > 0)
      {
        result += U", ";
      }
      result += m_elements[i]->to_string();
    }
    result += U']';

    return result;
  }

  RValue::result_type
  List::eval(Interpreter& interpreter, const Scope& scope) const
  {
    value::List::container_type elements;

    elements.reserve(m_elements.size());
    for (const auto& element : m_elements)
    {
      const auto result = element->eval(interpreter, scope);

     if (!result)
      {
        return result;
      }
      elements.push_back(result.value());
    }

    return result_type::ok(std::make_shared<value::List>(elements));
  }

  LValue::assign_result_type
  List::assign(
    Interpreter& interpreter,
    Scope& scope,
    const std::shared_ptr<value::Base>& value
  ) const
  {
    std::shared_ptr<value::List> value_list;

    if (value->kind() != value::Kind::List)
    {
      return assign_result_type({
        position(),
        U"Cannot assign " +
        value->type(interpreter)->to_string() +
        U" into an list."
      });
    }
    value_list = std::static_pointer_cast<value::List>(value);
    if (value_list->elements().size() < m_elements.size())
    {
      return assign_result_type({
        position(),
        U"List has too few elements for assignment."
      });
    }
    for (std::size_t i = 0; i < m_elements.size(); ++i)
    {
      const auto& target = m_elements[i];

      if (target->kind() == Kind::LValue)
      {
        const auto error = std::static_pointer_cast<LValue>(target)->assign(
          interpreter,
          scope,
          value_list->elements()[i]
        );

        if (error)
        {
          return error;
        }
      } else {
        return assign_result_type({
          target->position(),
          U"Cannot assign into `" +
          target->to_string() +
          U"'."
        });
      }
    }

    return assign_result_type();
  }
}
