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
#pragma once

#include <vector>

#include <peelo/result.hpp>

#include <snek/cst.hpp>
#include <snek/error.hpp>

namespace snek::ast { class Parameter; }
namespace snek::ast::expr { class RValue; }
namespace snek::ast::import { class Specifier; }
namespace snek::ast::record { class Field; }
namespace snek::ast::stmt { class Base; }
namespace snek::ast::type { class Base; }

namespace snek::parser
{
  using result_type = peelo::result<
    std::vector<std::shared_ptr<ast::stmt::Base>>,
    Error
  >;
  using parameter_result_type = peelo::result<
    std::shared_ptr<ast::Parameter>,
    Error
  >;
  using parameter_list_result_type = peelo::result<
    std::vector<std::shared_ptr<ast::Parameter>>,
    Error
  >;

  struct State
  {
    using iterator = std::vector<cst::Token>::const_iterator;
    iterator current;
    const iterator end;

    inline bool eof() const
    {
      return current >= end;
    }

    inline bool peek(cst::Kind kind, std::size_t offset = 0) const
    {
      return current + offset < end && (current + offset)->kind() == kind;
    }

    inline bool peek_read(cst::Kind kind)
    {
      if (peek(kind))
      {
        ++current;

        return true;
      }

      return false;
    }

    // Special hack that tries to recognize whether function expression is
    // coming next.
    inline bool peek_func() const
    {
      return peek(cst::Kind::LeftParen) && (
        // ():
        (
          peek(cst::Kind::RightParen, 1) &&
          peek(cst::Kind::Colon, 2)
        ) ||
        // () -> Type:
        (
          peek(cst::Kind::RightParen, 1) &&
          peek(cst::Kind::Arrow, 2)
        ) ||
        // (arg: Type):
        (
          peek(cst::Kind::Id, 1) &&
          peek(cst::Kind::Colon, 2)
        )
     );
    }
  };

  result_type parse(const std::vector<cst::Token>& tokens);

  parameter_result_type parse_parameter(State& state);

  parameter_list_result_type parse_parameter_list(State& state);
}

namespace snek::parser::expr
{
  using result_type = peelo::result<std::shared_ptr<ast::expr::RValue>, Error>;

  result_type parse(
    State& state,
    const std::optional<ast::Position>& position = std::nullopt
  );
  result_type parse_func(State& state);
}

namespace snek::parser::stmt
{
  using result_type = peelo::result<std::shared_ptr<ast::stmt::Base>, Error>;

  result_type parse(State& state);
  result_type parse_block(State& state, const ast::Position& position);
}

namespace snek::parser::import
{
  using result_type = peelo::result<
    std::shared_ptr<ast::import::Specifier>,
    Error
  >;

  result_type parse(State& state, const ast::Position& position);
}

namespace snek::parser::type
{
  using result_type = peelo::result<std::shared_ptr<ast::type::Base>, Error>;

  result_type parse(State& state, const ast::Position& position);
}

namespace snek::parser::record
{
  using result_type = peelo::result<
    std::shared_ptr<ast::record::Field>,
    Error
  >;

  result_type parse(State& state, const ast::Position& position);
}
