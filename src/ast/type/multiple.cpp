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
#include <snek/ast/type/multiple.hpp>
#include <snek/type/intersection.hpp>
#include <snek/type/tuple.hpp>
#include <snek/type/union.hpp>

namespace snek::ast::type
{
  Multiple::Multiple(
    const Position& position,
    const MultipleKind multiple_kind,
    const container_type& types
  )
    : Base(position)
    , m_multiple_kind(multiple_kind)
    , m_types(types) {}

  Base::result_type
  Multiple::eval(const Interpreter& interpreter, const Scope& scope) const
  {
    std::vector<std::shared_ptr<snek::type::Base>> types;

    types.reserve(m_types.size());
    for (const auto& type : m_types)
    {
      const auto result = type->eval(interpreter, scope);

      if (!result)
      {
        return result;
      }
      types.push_back(result.value());
    }
    switch (m_multiple_kind)
    {
      case MultipleKind::Intersection:
        return result_type::ok(std::make_shared<snek::type::Intersection>(
          types
        ));

      case MultipleKind::Tuple:
        return result_type::ok(std::make_shared<snek::type::Tuple>(types));

      case MultipleKind::Union:
        return result_type::ok(std::make_shared<snek::type::Union>(types));
    };
  }
}
