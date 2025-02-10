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

#include "snek/interpreter/config.hpp"
#include "snek/interpreter/error.hpp"
#include "snek/interpreter/scope.hpp"

namespace snek::interpreter
{
  Scope::ptr
  ImportFilesystemModule(
    const std::optional<Position>& position,
    Runtime&,
    const std::u32string& path
  );

  class Runtime
  {
  public:
#if defined(SNEK_ENABLE_INT_CACHE)
    static constexpr std::int64_t kIntCacheMin = -5;
    static constexpr std::int64_t kIntCacheMax = 256;
    static constexpr std::size_t kIntCacheSize = -kIntCacheMin + kIntCacheMax;
#endif

    using call_stack_type = std::stack<Frame>;
    using module_importer_type = std::function<
      Scope::ptr(
        const std::optional<Position>&,
        Runtime&,
        const std::u32string&
      )
    >;
    using module_container_type = std::unordered_map<
      std::u32string,
      Scope::ptr
    >;

    DEFAULT_COPY_AND_ASSIGN(Runtime);

    Runtime(
      const module_importer_type& module_importer = ImportFilesystemModule
    );

    inline const type::ptr& any_type() const
    {
      return m_any_type;
    }

    inline const type::ptr& boolean_type() const
    {
      return m_boolean_type;
    }

    inline const type::ptr& float_type() const
    {
      return m_float_type;
    }

    inline const type::ptr& function_type() const
    {
      return m_function_type;
    }

    inline const type::ptr& int_type() const
    {
      return m_int_type;
    }

    inline const type::ptr& list_type() const
    {
      return m_list_type;
    }

    inline const type::ptr& number_type() const
    {
      return m_number_type;
    }

    inline const type::ptr& record_type() const
    {
      return m_record_type;
    }

    inline const type::ptr& string_type() const
    {
      return m_string_type;
    }

    inline const type::ptr& void_type() const
    {
      return m_void_type;
    }

    inline const value::ptr& boolean_prototype() const
    {
      return m_boolean_prototype;
    }

    inline const value::ptr& float_prototype() const
    {
      return m_float_prototype;
    }

    inline const value::ptr& function_prototype() const
    {
      return m_function_prototype;
    }

    inline const value::ptr& int_prototype() const
    {
      return m_int_prototype;
    }

    inline const value::ptr& list_prototype() const
    {
      return m_list_prototype;
    }

    inline const value::ptr& number_prototype() const
    {
      return m_number_prototype;
    }

    inline const value::ptr& object_prototype() const
    {
      return m_object_prototype;
    }

    inline const value::ptr& record_prototype() const
    {
      return m_record_prototype;
    }

    inline const value::ptr& string_prototype() const
    {
      return m_string_prototype;
    }

    inline const Scope::ptr& root_scope() const
    {
      return m_root_scope;
    }

    inline call_stack_type& call_stack()
    {
      return m_call_stack;
    }

    inline const call_stack_type& call_stack() const
    {
      return m_call_stack;
    }

    inline value::ptr MakeBoolean(bool value)
    {
#if defined(SNEK_ENABLE_BOOLEAN_CACHE)
      return value ? m_true_value : m_false_value;
#else
      return std::make_shared<value::Boolean>(value);
#endif
    }

    /**
     * Constructs an error instance with copy of the runtime's current call
     * stack.
     */
    inline Error MakeError(const std::u32string& message) const
    {
      return Error{ m_call_stack, message };
    }

    value::ptr MakeInt(std::int64_t value);

    value::ptr RunScript(
      const Scope::ptr& scope,
      const std::string& source,
      const std::u32string& filename = U"<eval>",
      int line = 1,
      int column = 1
    );

    value::ptr RunScript(
      const Scope::ptr& scope,
      const std::u32string& source,
      const std::u32string& filename = U"<eval>",
      int line = 1,
      int column = 1
    );

    Scope::ptr ImportModule(
      const std::optional<Position>& position,
      const std::u32string& path
    );

  private:
    type::ptr m_any_type;
    type::ptr m_boolean_type;
    type::ptr m_float_type;
    type::ptr m_function_type;
    type::ptr m_int_type;
    type::ptr m_list_type;
    type::ptr m_number_type;
    type::ptr m_record_type;
    type::ptr m_string_type;
    type::ptr m_void_type;

    value::ptr m_object_prototype;
    value::ptr m_number_prototype;
    value::ptr m_boolean_prototype;
    value::ptr m_float_prototype;
    value::ptr m_function_prototype;
    value::ptr m_int_prototype;
    value::ptr m_list_prototype;
    value::ptr m_record_prototype;
    value::ptr m_string_prototype;

    Scope::ptr m_root_scope;

    call_stack_type m_call_stack;

    module_importer_type m_module_importer;
    module_container_type m_imported_modules;

#if defined(SNEK_ENABLE_BOOLEAN_CACHE)
    value::ptr m_true_value;
    value::ptr m_false_value;
#endif
#if defined(SNEK_ENABLE_INT_CACHE)
    value::ptr m_int_cache[kIntCacheSize];
#endif
  };
}
