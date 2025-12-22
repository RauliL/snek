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

namespace snek::interpreter
{
  Parameter
  ResolveParameter(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const parser::Parameter& parameter
  )
  {
    const auto type = ResolveType(runtime, scope, parameter.type);

    if (parameter.default_value)
    {
      const auto default_value_type = ResolveExpression(
        runtime,
        scope,
        parameter.default_value
      );

      if (!type->Accepts(default_value_type))
      {
        throw runtime.MakeError(
          U"Parameter default value type "
          + default_value_type->ToString()
          + U" does not match parameter type "
          + type->ToString()
          + U"."
        );
      }
    }

    return {
      parameter.name,
      type,
      parameter.default_value,
      parameter.rest,
      parameter.position
    };
  }

  std::vector<Parameter>
  ResolveParameterList(
    const Runtime& runtime,
    const Scope::ptr& scope,
    const std::vector<parser::Parameter>& parameters
  )
  {
    std::vector<Parameter> result;

    result.reserve(parameters.size());
    for (const auto& parameter : parameters)
    {
      result.push_back(ResolveParameter(runtime, scope, parameter));
    }

    return result;
  }
}
