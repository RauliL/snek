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
#pragma once

#include "snek/interpreter/runtime.hpp"
#include "snek/interpreter/type.hpp"
#include "snek/parser/expression.hpp"
#include "snek/parser/parameter.hpp"
#include "snek/parser/statement.hpp"
#include "snek/parser/type.hpp"

namespace snek::interpreter
{
  type::ptr
  ResolveType(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::type::ptr& type
  );

  Parameter
  ResolveParameter(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::parameter::ptr& parameter
  );

  std::vector<Parameter>
  ResolveParameterList(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const std::vector<parser::parameter::ptr>& parameters
  );

  type::ptr
  ResolveExpression(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::expression::ptr& expression
  );

  type::ptr
  ResolveStatement(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::statement::ptr& statement
  );

  bool
  ResolveField(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::field::ptr& field,
    type::Record::container_type& resolved_fields
  );
}
