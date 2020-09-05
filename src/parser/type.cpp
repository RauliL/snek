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
#include <snek/ast/type/builtin.hpp>
#include <snek/ast/type/func.hpp>
#include <snek/ast/type/intersection.hpp>
#include <snek/ast/type/list.hpp>
#include <snek/ast/type/named.hpp>
#include <snek/ast/type/record.hpp>
#include <snek/ast/type/str.hpp>
#include <snek/ast/type/tuple.hpp>
#include <snek/ast/type/union.hpp>
#include <snek/parser.hpp>

namespace snek::parser::type
{
  using builtin_types_mapping = std::unordered_map<
    std::u32string,
    ast::type::BuiltinKind
  >;

  static const builtin_types_mapping builtin_types =
  {
    { U"Any", ast::type::BuiltinKind::Any },
    { U"Bool", ast::type::BuiltinKind::Bool },
    { U"Float", ast::type::BuiltinKind::Float },
    { U"Int", ast::type::BuiltinKind::Int },
    { U"Num", ast::type::BuiltinKind::Num },
    { U"Str", ast::type::BuiltinKind::Str },
    { U"Void", ast::type::BuiltinKind::Void },
  };

  static result_type
  parse_named_type(State& state)
  {
    const auto position = state.current->position();
    const auto name = *state.current++->text();
    const auto builtin_type = builtin_types.find(name);

    if (builtin_type != std::end(builtin_types))
    {
      return result_type::ok(std::make_shared<ast::type::Builtin>(
        position,
        builtin_type->second
      ));
    }

    return result_type::ok(std::make_shared<ast::type::Named>(
      position,
      name
    ));
  }

  static result_type
  parse_func_type(State& state)
  {
    const auto position = state.current->position();
    std::vector<std::shared_ptr<ast::Parameter>> parameters;
    std::optional<std::shared_ptr<ast::type::Base>> return_type;

    if (const auto error = parse_parameter_list(state, parameters, position))
    {
      return result_type::error(*error);
    }
    if (state.peek_read(cst::Kind::Arrow))
    {
      const auto result = type::parse(state, position);

      if (!result)
      {
        return result_type::error(result.error());
      }
      return_type = result.value();
    }

    return result_type::ok(std::make_shared<ast::type::Func>(
      position,
      parameters,
      return_type
    ));
  }

  static result_type
  parse_tuple_type(State& state)
  {
    const auto position = state.current++->position();
    ast::type::Tuple::container_type types;

    for (;;)
    {
      if (state.eof())
      {
        return result_type::error({
          position,
          U"Unterminated tuple type; Missing `]'."
        });
      }
      else if (state.peek_read(cst::Kind::RightBracket))
      {
        break;
      } else {
        const auto type = parse(state, position);

        if (!type)
        {
          return type;
        }
        types.push_back(type.value());
        if (!state.peek(cst::Kind::Comma) &&
            !state.peek(cst::Kind::RightBracket))
        {
          return result_type::error({
            type.value()->position(),
            U"Unterminated tuple type; Missing `]'."
          });
        }
        state.peek_read(cst::Kind::Comma);
      }
    }

    return result_type::ok(std::make_shared<ast::type::Tuple>(
      position,
      types
    ));
  }

  static result_type
  parse_record_type(State& state)
  {
    const auto position = state.current++->position();
    ast::type::Record::container_type fields;

    for (;;)
    {
      if (state.eof())
      {
        return result_type::error({
          position,
          U"Unterminated record type; Missing `}'."
        });
      }
      else if (state.peek_read(cst::Kind::RightBrace))
      {
        break;
      } else {
        std::u32string name;

        if (!state.peek(cst::Kind::Id) && !state.peek(cst::Kind::Str))
        {
          return result_type::error({
            position,
            U"Unexpected " +
            cst::to_string(state.current->kind()) +
            U"; Missing identifier."
          });
        }
        name = *state.current++->text();
        if (!state.peek_read(cst::Kind::Colon))
        {
          return result_type::error({
            position,
            U"Missing `:' after record field name."
          });
        }

        const auto type = parse(state, position);

        if (!type)
        {
          return type;
        }

        fields[name] = type.value();
        if (!state.peek(cst::Kind::Comma) &&
            !state.peek(cst::Kind::RightBrace))
        {
          return result_type::error({
            type.value()->position(),
            U"Unterminated record type; Missing `}'."
          });
        }
        state.peek_read(cst::Kind::Comma);
      }
    }

    return result_type::ok(std::make_shared<ast::type::Record>(
      position,
      fields
    ));
  }

  static result_type
  parse_intersection_type(
    State& state,
    const std::shared_ptr<ast::type::Base>& first_type
  )
  {
    const auto position = state.current++->position();
    ast::type::Intersection::container_type types;

    types.push_back(first_type);
    for (;;)
    {
      if (state.eof())
      {
        return result_type::error({
          position,
          U"Unexpected end of input; Missing type."
        });
      }

      const auto type = parse(state, position);

      if (!type)
      {
        return type;
      }

      types.push_back(type.value());

      if (!state.peek_read(cst::Kind::And))
      {
        break;
      }
    }

    return result_type::ok(std::make_shared<ast::type::Intersection>(
      position,
      types
    ));
  }

  static result_type
  parse_union_type(
    State& state,
    const std::shared_ptr<ast::type::Base>& first_type
  )
  {
    const auto position = state.current++->position();
    ast::type::Union::container_type types;

    types.push_back(first_type);
    for (;;)
    {
      if (state.eof())
      {
        return result_type::error({
          position,
          U"Unexpected end of input; Missing type."
        });
      }

      const auto type = parse(state, position);

      if (!type)
      {
        return type;
      }

      types.push_back(type.value());

      if (!state.peek_read(cst::Kind::Or))
      {
        break;
      }
    }

    return result_type::ok(std::make_shared<ast::type::Union>(
      position,
      types
    ));
  }

  result_type
  parse(State& state, const std::optional<ast::Position>& position)
  {
    std::shared_ptr<ast::type::Base> type;

    if (state.eof())
    {
      return result_type::error({
        position,
        U"Unexpected end of input; Missing type."
      });
    }
    switch (state.current->kind())
    {
      case cst::Kind::Id:
        {
          const auto result = parse_named_type(state);

          if (!result)
          {
            return result;
          }
          type = result.value();
        }
        break;

      case cst::Kind::LeftParen:
        {
          const auto result = parse_func_type(state);

          if (!result)
          {
            return result;
          }
          type = result.value();
        }
        break;

      case cst::Kind::LeftBracket:
        {
          const auto result = parse_tuple_type(state);

          if (!result)
          {
            return result;
          }
          type = result.value();
        }
        break;

      case cst::Kind::LeftBrace:
        {
          const auto result = parse_record_type(state);

          if (!result)
          {
            return result;
          }
          type = result.value();
        }
        break;

      case cst::Kind::Str:
        type = std::make_shared<ast::type::Str>(
          state.current->position(),
          *state.current++->text()
        );
        break;

      default:
        return result_type::error({
          state.current->position(),
          U"Unexpected " +
          cst::to_string(state.current->kind()) +
          U"; Missing type."
        });
    }
    while (state.peek_read(cst::Kind::LeftBracket))
    {
      if (!state.peek_read(cst::Kind::RightBracket))
      {
        return result_type::error({
          type->position(),
          U"Missing `]' after `['."
        });
      }
      type = std::make_shared<ast::type::List>(type->position(), type);
    }
    if (state.peek(cst::Kind::And))
    {
      return parse_intersection_type(state, type);
    }
    else if (state.peek(cst::Kind::Or))
    {
      return parse_union_type(state, type);
    }

    return result_type::ok(type);
  }
}
