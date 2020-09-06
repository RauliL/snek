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
#include <snek/ast/expr/base.hpp>
#include <snek/ast/stmt/export.hpp>
#include <snek/ast/type/base.hpp>
#include <snek/scope.hpp>

namespace snek::ast::stmt
{
  ExportExpr::ExportExpr(
    const Position& position,
    const std::u32string& name,
    const std::shared_ptr<expr::RValue>& expr
  )
    : Base(position)
    , m_name(name)
    , m_expr(expr) {}

  std::u32string
  ExportExpr::to_string() const
  {
    return U"export " + m_name + U" = " + m_expr->to_string();
  }

  void
  ExportExpr::exec(
    Interpreter& interpreter,
    Scope& scope,
    ExecContext& context
  ) const
  {
    const auto result = m_expr->eval(interpreter, scope);

    if (!result)
    {
      context.error() = result.error();
    }
    else if (!scope.add_variable(m_name, result.value()))
    {
      context.error() = {
        position(),
        U"Variable `" + m_name + U"' has already been defined."
      };
    }
    else if (!scope.add_exported_variable(m_name, result.value()))
    {
      context.error() = {
        position(),
        U"Export `" + m_name + U"' has already been defined."
      };
    }
  }

  ExportType::ExportType(
    const Position& position,
    const std::u32string& name,
    const std::shared_ptr<type::Base>& type
  )
    : Base(position)
    , m_name(name)
    , m_type(type) {}

  std::u32string
  ExportType::to_string() const
  {
    return U"export type " + m_name + U" = " + m_type->to_string();
  }

  void
  ExportType::exec(
    Interpreter& interpreter,
    Scope& scope,
    ExecContext& context
  ) const
  {
    const auto result = m_type->eval(interpreter, scope);

    if (!result)
    {
      context.error() = result.error();
    }
    else if (!scope.add_type(m_name, result.value()))
    {
      context.error() = {
        position(),
        U"Type `" + m_name + U"' has already been defined."
      };
    }
    else if (!scope.add_exported_type(m_name, result.value()))
    {
      context.error() = {
        position(),
        U"Export `" + m_name + U"' has already been defined."
      };
    }
  }

  ExportName::ExportName(const Position& position, const std::u32string& name)
    : Base(position)
    , m_name(name) {}

  std::u32string
  ExportName::to_string() const
  {
    return U"export " + m_name;
  }

  void
  ExportName::exec(
    Interpreter& interpreter,
    Scope& scope,
    ExecContext& context
  ) const
  {
    if (const auto value = scope.find_variable(m_name))
    {
      if (!scope.add_exported_variable(m_name, *value))
      {
        context.error() = {
          position(),
          U"Export `" + m_name + U"' has already been defined."
        };
      }
    }
    else if (const auto type = scope.find_type(m_name))
    {
      if (!scope.add_exported_type(m_name, *type))
      {
        context.error() = {
          position(),
          U"Export `" + m_name + U"' has already been defined."
        };
      }
    } else {
      context.error() = {
        position(),
        U"Unknown variable or type: `" + m_name + U"'."
      };
    }
  }
}
