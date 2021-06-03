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

#include <peelo/result.hpp>

#include <snek/error.hpp>
#include <snek/scope.hpp>
#include <snek/type/base.hpp>
#include <snek/value/bool.hpp>
#include <snek/value/null.hpp>
#include <snek/value/record.hpp>

namespace snek
{
  class Interpreter
  {
  public:
    using module_type = value::RecordPtr;
    using module_container_type = std::unordered_map<std::u32string, Scope>;
    using module_import_result = peelo::result<const Scope, Error>;
    using eval_result_type = peelo::result<value::Ptr, Error>;

    explicit Interpreter();
    Interpreter(const Interpreter&) = default;
    Interpreter(Interpreter&&) = default;
    Interpreter& operator=(const Interpreter&) = default;
    Interpreter& operator=(Interpreter&&) = default;

    inline const type::Ptr& any_type() const
    {
      return m_any_type;
    }

    inline const type::Ptr& bin_type() const
    {
      return m_bin_type;
    }

    inline const type::Ptr& bool_type() const
    {
      return m_bool_type;
    }

    inline const type::Ptr& float_type() const
    {
      return m_float_type;
    }

    inline const type::Ptr& int_type() const
    {
      return m_int_type;
    }

    inline const type::Ptr& num_type() const
    {
      return m_num_type;
    }

    inline const type::Ptr& record_type() const
    {
      return m_record_type;
    }

    inline const type::Ptr& str_type() const
    {
      return m_str_type;
    }

    inline const type::Ptr& void_type() const
    {
      return m_void_type;
    }

    inline const value::NullPtr& null_value() const
    {
      return m_null_value;
    }

    inline const value::BoolPtr& true_value() const
    {
      return m_true_value;
    }

    inline const value::BoolPtr& false_value() const
    {
      return m_false_value;
    }

    inline const value::BoolPtr& bool_value(bool value) const
    {
      return value ? m_true_value : m_false_value;
    }

    std::optional<Error> exec(
      const std::u32string& source,
      const std::u32string& file = U"<eval>",
      int line = 1,
      int column = 0
    );

    eval_result_type eval(
      const std::u32string& source,
      Scope& scope,
      const std::u32string& file = U"<eval>",
      int line = 1,
      int column = 0
    );

    module_import_result import_module(
      const std::u32string& path,
      const std::optional<ast::Position>& position = std::nullopt
    );

  private:
    module_container_type m_modules;
    type::Ptr m_any_type;
    type::Ptr m_bin_type;
    type::Ptr m_bool_type;
    type::Ptr m_float_type;
    type::Ptr m_int_type;
    type::Ptr m_num_type;
    type::Ptr m_record_type;
    type::Ptr m_str_type;
    type::Ptr m_void_type;
    value::NullPtr m_null_value;
    value::BoolPtr m_true_value;
    value::BoolPtr m_false_value;
  };
}
