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
#include <cctype>
#include <fstream>

#include <peelo/unicode/ctype/isspace.hpp>
#include <peelo/unicode/encoding/utf8.hpp>

#include <snek/config.hpp>

#if SNEK_HAS_SYS_TYPES_H
# include <sys/types.h>
#endif
#if SNEK_HAS_SYS_STAT_H
# include <sys/stat.h>
#endif
#if SNEK_HAS_UNISTD_H
# include <unistd.h>
#endif

#include <snek/ast/stmt/base.hpp>
#include <snek/interpreter.hpp>
#include <snek/lexer.hpp>
#include <snek/parser.hpp>
#include <snek/type/utils.hpp>
#include <snek/value/utils.hpp>

#if !defined(PATH_MAX)
# define PATH_MAX 1024
#endif

namespace snek
{
  using namespace snek::type::utils;
  using namespace snek::value::utils;

  namespace api::bin { Scope create(const Interpreter&); }
  namespace api::debug { Scope create(const Interpreter&); }
  namespace api::io { Scope create(const Interpreter&); }
  namespace api::list { Scope create(const Interpreter&); }
  namespace api::record { Scope create(const Interpreter&); }
  namespace api::str { Scope create(const Interpreter&); }

#if defined(_WIN32)
  static const char file_separator = '\\';
#else
  static const char file_separator = '/';
#endif

  static std::optional<std::u32string> resolve_path(const std::u32string&);

  Interpreter::Interpreter()
    : m_any_type(make_any_type())
    , m_bin_type(make_primitive_type(type::PrimitiveKind::Bin))
    , m_bool_type(make_primitive_type(type::PrimitiveKind::Bool))
    , m_float_type(make_primitive_type(type::PrimitiveKind::Float))
    , m_int_type(make_primitive_type(type::PrimitiveKind::Int))
    , m_num_type(make_primitive_type(type::PrimitiveKind::Num))
    , m_record_type(make_primitive_type(type::PrimitiveKind::Record))
    , m_str_type(make_primitive_type(type::PrimitiveKind::Str))
    , m_void_type(make_primitive_type(type::PrimitiveKind::Void))
    , m_null_value(std::make_shared<value::Null>())
    , m_true_value(std::make_shared<value::Bool>(true))
    , m_false_value(std::make_shared<value::Bool>(false))
  {
    m_modules[U"bin"] = api::bin::create(*this);
    m_modules[U"debug"] = api::debug::create(*this);
    m_modules[U"io"] = api::io::create(*this);
    m_modules[U"list"] = api::list::create(*this);
    m_modules[U"record"] = api::record::create(*this);
    m_modules[U"str"] = api::str::create(*this);
  }

  static parser::result_type
  lex_and_parse(
    const std::u32string& source,
    const std::u32string& file,
    int line = 1,
    int column = 1
  )
  {
    const auto tokens = lexer::lex(source, file, line, column);

    if (!tokens)
    {
      return parser::result_type::error(tokens.error());
    }

    return parser::parse(tokens.value());
  }

  std::optional<Error>
  Interpreter::exec(
    const std::u32string& source,
    const std::u32string& file,
    int line,
    int column
  )
  {
    const auto nodes = lex_and_parse(source, file, line, column);
    Scope scope;
    ast::stmt::ExecContext context;

    if (!nodes)
    {
      return nodes.error();
    }
    for (const auto& node : nodes.value())
    {
      node->exec(*this, scope, context);
      if (context.error())
      {
        return context.error();
      }
      // TODO: Handle dangling break, continue and return.
    }

    return std::nullopt;
  }

  Interpreter::eval_result_type
  Interpreter::eval(
    const std::u32string& source,
    Scope& scope,
    const std::u32string& file,
    int line,
    int column
  )
  {
    const auto nodes = lex_and_parse(source, file, line, column);
    ast::stmt::ExecContext context;

    if (!nodes)
    {
      return eval_result_type::error(nodes.error());
    }
    for (const auto& node : nodes.value())
    {
      node->exec(*this, scope, context);
      if (context.error())
      {
        return eval_result_type::error(*context.error());
      }
      // TODO: Handle dangling break, continue and return.
    }

    return eval_result_type::ok(
      context.value() ? context.value() : m_null_value
    );
  }

  Interpreter::module_import_result
  Interpreter::import_module(
    const std::u32string& path,
    const std::optional<ast::Position>& position
  )
  {
    using peelo::unicode::ctype::isspace;
    using peelo::unicode::encoding::utf8::decode;
    using peelo::unicode::encoding::utf8::decode_validate;
    using peelo::unicode::encoding::utf8::encode;
    const auto module = m_modules.find(path);

    if (module != std::end(m_modules))
    {
      return module_import_result::ok(module->second);
    }

    // Do not attempt to import empty paths.
    if (path.empty() || std::all_of(std::begin(path), std::end(path), isspace))
    {
      return module_import_result::error({
        position,
        U"Cannot import empty path."
      });
    }

    // Lets see if the given path actually resolves into actual file on the
    // file system.
    if (const auto resolved_path = resolve_path(path))
    {
      std::ifstream is(encode(*resolved_path));
      std::string raw_source;
      std::u32string source;
      Scope module_scope;

      if (!is.good())
      {
        return module_import_result::error({
          position,
          U"Unable to read from `" +
          *resolved_path +
          U"'."
        });
      }

      raw_source = std::string(
        std::istreambuf_iterator<char>(is),
        std::istreambuf_iterator<char>()
      );
      is.close();

      if (!decode_validate(raw_source, source))
      {
        return module_import_result::error({
          position,
          U"Unable to decode source code into UTF-8."
        });
      }

      const auto nodes = lex_and_parse(source, *resolved_path);
      ast::stmt::ExecContext context;

      if (!nodes)
      {
        return module_import_result::error(nodes.error());
      }
      for (const auto& node : nodes.value())
      {
        node->exec(*this, module_scope, context);
        if (context.error())
        {
          return module_import_result::error(*context.error());
        }
        // TODO: Handle dangling break, continue and return.
      }

      m_modules[*resolved_path] = module_scope;

      return module_import_result::ok(module_scope);
    }

    return module_import_result::error({
      position,
      U"No such file or directory: " + path
    });
  }

  /**
   * Tests whether given file path appears to be non-relative and pointing
   * directly to a specific file in the file system.
   */
  static bool is_absolute_path(const std::u32string& path)
  {
    const auto length = path.length();

    // Does the path begin file separator?
    if (length > 0 && path[0] == file_separator)
    {
      return true;
    }
#if defined(_WIN32)
    // Does the path begin with drive letter on Windows?
    else if (length > 3 &&
             std::isalpha(path[0]) &&
             path[1] == ':' &&
             path[2] == file_separator)
    {
      return true;
    }
#endif
    // Does the path begin with './'?
    if (length > 1 && path[0] == '.' && path[1] == file_separator)
    {
      return true;
    }
    // Does the path begin with '../'?
    else if (length > 2 &&
             path[0] == '.' &&
             path[1] == '.' &&
             path[2] == file_separator)
    {
      return true;
    }

    return false;
  }

  static std::optional<std::u32string>
  resolve_into_file(const std::string& path)
  {
    using peelo::unicode::encoding::utf8::decode;
    struct ::stat st;

    // Does the path even exist in the file system?
    if (::stat(path.c_str(), &st) < 0)
    {
      return std::nullopt;
    }

    // Is it an directory? If so, look up for a file called "index.snek" from
    // it.
    if (S_ISDIR(st.st_mode))
    {
      auto index_file_path = path;

      if (index_file_path.back() != file_separator)
      {
        index_file_path += file_separator;
      }
      index_file_path += "index.snek";

      // OK. Does the index file exist?
      if (::stat(index_file_path.c_str(), &st) >= 0 && S_ISREG(st.st_mode))
      {
        return decode(index_file_path);
      }
    }
    // Is it ordinary file? Then use that one.
    else if (S_ISREG(st.st_mode))
    {
      return decode(path);
    }

    // Otherwise give up.
    return std::nullopt;
  }

  static std::optional<std::u32string>
  resolve_path(const std::u32string& path)
  {
    using peelo::unicode::encoding::utf8::encode;
    auto encoded_path = encode(path);
    char buffer[PATH_MAX];

    // If the path is absolute, resolve it into full path and use that
    // directly, otherwise assume that the path is relative path.
    if (!is_absolute_path(path))
    {
      encoded_path.insert(0, 1, file_separator);
      encoded_path.insert(0, 1, U'.');
    }
    if (!::realpath(encoded_path.c_str(), buffer))
    {
      // Try again with appended file extension.
      encoded_path += ".snek";
      if (!::realpath(encoded_path.c_str(), buffer))
      {
        // Give up.
        return std::nullopt;
      }
    }

    return resolve_into_file(buffer);
  }
}
