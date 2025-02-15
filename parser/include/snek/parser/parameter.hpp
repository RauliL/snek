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

#include <memory>
#include <vector>

#include "snek/parser/lexer.hpp"

namespace snek::parser::expression
{
  class Base;

  using ptr = std::shared_ptr<Base>;
}

namespace snek::parser::type
{
  class Base;

  using ptr = std::shared_ptr<Base>;
}

namespace snek::parser
{
  class Parameter final : public Node
  {
  public:
    static Parameter Parse(Lexer& lexer);

    static std::vector<Parameter> ParseList(
      Lexer& lexer,
      bool read_opening_parenthesis = true
    );

    const std::u32string name;
    const type::ptr type;
    const expression::ptr default_value;
    const bool rest;

    Parameter(
      const std::optional<Position>& position = std::nullopt,
      const std::u32string& name_ = U"arg",
      const type::ptr& type_ = nullptr,
      const expression::ptr& default_value_ = nullptr,
      bool rest_ = false
    )
      : Node(position)
      , name(name_)
      , type(type_)
      , default_value(default_value_)
      , rest(rest_) {}

    std::u32string ToString() const override;
  };
}
