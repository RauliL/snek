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
#include "snek/interpreter/error.hpp"
#include "snek/interpreter/evaluate.hpp"
#include "snek/interpreter/execute.hpp"
#include "snek/interpreter/jump.hpp"
#include "snek/interpreter/value.hpp"

namespace snek::interpreter::value
{
  using argument_callback_type = std::function<void(
    const Parameter&,
    const value::ptr&
  )>;

  static void
  ProcessArguments(
    Runtime& runtime,
    const Scope::ptr& scope,
    const std::vector<Parameter>& parameters,
    const std::vector<value::ptr>& arguments,
    argument_callback_type callback
  )
  {
    const auto parameters_size = parameters.size();
    const auto arguments_size = arguments.size();

    for (std::size_t i = 0; i < parameters_size; ++i)
    {
      const auto& parameter = parameters[i];
      ptr argument;

      if (parameter.rest)
      {
        std::vector<value::ptr> elements;

        for (std::size_t j = i; j < arguments_size; ++j)
        {
          // TODO: Use insert instead.
          elements.push_back(arguments[j]);
        }
        argument = value::List::Make(elements);
        i = parameters_size;
      }
      else if (i < arguments_size)
      {
        argument = arguments[i];
      }
      else if (parameter.default_value)
      {
        argument = EvaluateExpression(runtime, scope, parameter.default_value);
      } else {
        throw runtime.MakeError(U"Too few arguments.");
      }
      if (!parameter.Accepts(runtime, argument))
      {
        throw runtime.MakeError(
          value::ToString(argument) +
          U" cannot be assigned to " +
          parameter.ToString()
        );
      }
      callback(parameter, argument);
    }
  }

  namespace
  {
    class NativeFunction final : public Function
    {
    public:
      explicit NativeFunction(
        const std::vector<Parameter>& parameters,
        const type::ptr& return_type,
        const callback_type& callback
      )
        : Function()
        , m_parameters(parameters)
        , m_return_type(return_type)
        , m_callback(callback) {}

      inline const std::vector<Parameter>& parameters() const override
      {
        return m_parameters;
      }

      inline const type::ptr& return_type() const override
      {
        return m_return_type;
      }

    protected:
      ptr
      Call(
        Runtime& runtime,
        const std::vector<ptr>& arguments,
        const std::optional<Position>&
      ) const override
      {
        std::vector<ptr> callback_arguments;

        callback_arguments.reserve(m_parameters.size());
        ProcessArguments(
          runtime,
          runtime.root_scope(),
          m_parameters,
          arguments,
          [&callback_arguments](const Parameter&, const value::ptr& argument)
          {
            callback_arguments.push_back(argument);
          }
        );

        return m_callback(runtime, callback_arguments);
      }

    private:
      const std::vector<Parameter> m_parameters;
      const type::ptr m_return_type;
      const callback_type m_callback;
    };

    class ScriptedFunction final : public Function
    {
    public:
      explicit ScriptedFunction(
        const std::vector<Parameter>& parameters,
        const type::ptr& return_type,
        const parser::statement::ptr& body,
        const Scope::ptr& enclosing_scope
      )
        : Function()
        , m_parameters(parameters)
        , m_return_type(return_type)
        , m_body(body)
        , m_enclosing_scope(enclosing_scope) {}

      inline const std::vector<Parameter>& parameters() const override
      {
        return m_parameters;
      }

      inline const type::ptr& return_type() const override
      {
        return m_return_type;
      }

    protected:
      ptr
      Call(
        Runtime& runtime,
        const std::vector<ptr>& arguments,
        const std::optional<Position>&
      ) const override
      {
        const auto scope = std::make_shared<Scope>(
          m_enclosing_scope
            ? m_enclosing_scope
            : runtime.root_scope()
        );

        ProcessArguments(
          runtime,
          scope,
          m_parameters,
          arguments,
          [&scope](const Parameter& parameter, const value::ptr& argument)
          {
            scope->DeclareVariable(parameter.name, argument, false);
          }
        );
        try
        {
          ExecuteStatement(runtime, scope, m_body);
        }
        catch (const Jump& jump)
        {
          if (jump.kind() == parser::statement::JumpKind::Return)
          {
            return jump.value();
          }

          throw runtime.MakeError(
            U"Unexpected `" +
            parser::statement::Jump::ToString(jump.kind())
            + U"'."
          );
        }

        return nullptr;
      }

    private:
      const std::vector<Parameter> m_parameters;
      const type::ptr m_return_type;
      const parser::statement::ptr m_body;
      const Scope::ptr m_enclosing_scope;
    };

    class BoundFunction final : public Function
    {
    public:
      explicit BoundFunction(
        const ptr& this_value,
        const std::shared_ptr<Function>& function
      )
        : Function()
        , m_this_value(this_value)
        , m_function(function)
        , m_parameters(
            function->parameters().empty()
              ? std::vector<Parameter>()
              : std::vector<Parameter>(
                  std::begin(function->parameters()) + 1,
                  std::end(function->parameters())
                )
          ) {}

      inline const std::vector<Parameter>& parameters() const override
      {
        return m_parameters;
      }

      inline const type::ptr& return_type() const override
      {
        return m_function->return_type();
      }

    protected:
      inline ptr
      Call(
        Runtime& runtime,
        const std::vector<ptr>& arguments,
        const std::optional<Position>& position
      ) const override
      {
        std::vector<ptr> bound_arguments(arguments);

        bound_arguments.insert(std::begin(bound_arguments), m_this_value);

        return Function::Call(
          runtime,
          m_function,
          bound_arguments,
          true,
          position
        );
      }

    private:
      const ptr m_this_value;
      const std::shared_ptr<Function> m_function;
      const std::vector<Parameter> m_parameters;
    };
  }

  std::shared_ptr<Function>
  Function::MakeNative(
    const std::vector<Parameter>& parameters,
    const type::ptr& return_type,
    const callback_type& callback
  )
  {
    return std::make_shared<NativeFunction>(
      parameters,
      return_type,
      callback
    );
  }

  std::shared_ptr<Function>
  Function::MakeScripted(
    const std::vector<Parameter>& parameters,
    const type::ptr& return_type,
    const parser::statement::ptr& body,
    const Scope::ptr& enclosing_scope
  )
  {
    return std::make_shared<ScriptedFunction>(
      parameters,
      return_type,
      body,
      enclosing_scope
    );
  }

  std::shared_ptr<Function>
  Function::Bind(
    const ptr& this_value,
    const std::shared_ptr<Function>& function
  )
  {
    return std::make_shared<BoundFunction>(this_value, function);
  }

  ptr
  Function::Call(
    Runtime& runtime,
    const std::shared_ptr<Function>& function,
    const std::vector<ptr>& arguments,
    bool tail_call,
    const std::optional<Position>& position
  )
  {
    auto& call_stack = runtime.call_stack();
    const auto use_tail = tail_call && !call_stack.empty();
    ptr value;

    if (use_tail)
    {
      auto& frame = call_stack.top();

      frame.function = function;
      frame.arguments = arguments;
    } else {
      call_stack.push({ position, function, arguments });
    }
    try
    {
      value = function->Call(runtime, arguments, position);
    }
    catch (const Error& e)
    {
      if (!use_tail)
      {
        call_stack.pop();
      }

      throw e;
    }
    if (!use_tail)
    {
      call_stack.pop();
    }

    return value;
  }

  bool
  Function::Equals(const Base& that) const
  {
    if (that.kind() == Kind::Function)
    {
      // TODO: Parameter and return type comparison.
      return this == static_cast<const Function*>(&that);
    }

    return false;
  }

  std::u32string
  Function::ToString() const
  {
    bool first = true;
    std::u32string result(1, U'(');

    for (const auto& parameter : parameters())
    {
      if (first)
      {
        first = false;
      } else {
        result.append(U", ");
      }
      result.append(parameter.ToString());
    }
    result.append(U") => ");
    if (const auto rtype = return_type())
    {
      result.append(rtype->ToString());
    } else {
      result.append(U"any");
    }

    return result;
  }
}
