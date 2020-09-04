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
#include <snek/ast/stmt/base.hpp>
#include <snek/interpreter.hpp>
#include <snek/message.hpp>
#include <snek/type/func.hpp>
#include <snek/value/func.hpp>

namespace snek::value
{
  Func::Func(
    const std::vector<Parameter>& parameters,
    const body_type& body,
    const std::optional<type::Ptr>& return_type,
    const std::optional<Scope>& enclosing_scope
  )
    : m_parameters(parameters)
    , m_body(body)
    , m_return_type(return_type)
    , m_enclosing_scope(enclosing_scope) {}

  Func::result_type
  Func::send(
    Interpreter& interpreter,
    const Message& message,
    const std::optional<ast::Position>& position
  ) const
  {
    const auto& args = message.args();

    if (m_parameters.size() > args.size())
    {
      return result_type::error({
        position,
        U"Not enough arguments."
      });
    }
    for (std::size_t i = 0; i < m_parameters.size(); ++i)
    {
      const auto& param = m_parameters[i];
      const auto& arg = args[i];

      if (!param.type()->matches(arg))
      {
        return result_type::error({
          position,
          arg->type(interpreter)->to_string() +
          U" cannot be assigned to " +
          param.type()->to_string() +
          U"."
        });
      }
    }
    if (!std::holds_alternative<callback_type>(m_body))
    {
      const auto body = std::get<std::shared_ptr<ast::stmt::Base>>(m_body);
      Scope scope(m_enclosing_scope
        ? std::make_shared<Scope>(*m_enclosing_scope)
        : nullptr
      );
      ast::stmt::ExecContext context;

      for (std::size_t i = 0; i < m_parameters.size(); ++i)
      {
        if (!scope.add_variable(m_parameters[i].name(), args[i]))
        {
          return result_type::error({
            body->position(),
            U"Variable `" +
            m_parameters[i].name() +
            U"' has already been defined."
          });
        }
      }
      body->exec(interpreter, scope, context);
      if (context.error())
      {
        return result_type::error(*context.error());
      }
      else if (context.jump() == ast::stmt::Jump::Return)
      {
        const auto value = context.value();

        return result_type::ok(value ? value : interpreter.null_value());
      }
      else if (context.jump() == ast::stmt::Jump::Break)
      {
        return result_type::error({
          body->position(),
          U"Unexpected `break'."
        });
      }
      else if (context.jump() == ast::stmt::Jump::Continue)
      {
        return result_type::error({
          body->position(),
          U"Unexpected `continue'."
        });
      }

      return result_type::ok(interpreter.null_value());
    }

    return std::get<callback_type>(m_body)(interpreter, message);
  }

  std::shared_ptr<type::Base>
  Func::type(const Interpreter& interpreter) const
  {
    return std::make_shared<type::Func>(
      m_parameters,
      m_return_type ? *m_return_type : interpreter.any_type()
    );
  }

  bool
  Func::equals(const Ptr& that) const
  {
    if (that->kind() == Kind::Func)
    {
      // TODO: Should we compare parameters and return types instead?
      return this == std::static_pointer_cast<Func>(that).get();
    }

    return false;
  }

  std::u32string
  Func::to_string() const
  {
    bool first = true;
    std::u32string result;

    result += U"(";
    for (const auto& parameter : m_parameters)
    {
      if (first)
      {
        first = false;
      } else {
        result += U", ";
      }
      result += parameter.to_string();
    }
    result += U") -> ";
    result += m_return_type ? (*m_return_type)->to_string() : U"Any";

    return result;
  }
}
