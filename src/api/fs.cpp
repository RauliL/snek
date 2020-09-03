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
#include <cstring>

#include <snek/api.hpp>
#include <snek/config.hpp>
#include <snek/interpreter.hpp>
#include <snek/message.hpp>
#include <snek/type/utils.hpp>
#include <snek/value/utils.hpp>

#include <dirent.h>
#if SNEK_HAS_UNISTD_H
# include <unistd.h>
#endif

#if !defined(MAX_PATH)
# define MAX_PATH 1024
#endif

namespace snek::api::fs
{
  using namespace snek::value::utils;
  using result_type = value::Func::result_type;

  /**
   * Changes the current working directory to given path. If the directory does
   * not exist, or isn't otherwise accessible, false will be returned,
   * otherwise true if returned.
   */
  static result_type
  func_chdir(Interpreter& interpreter, const Message& message)
  {
    using peelo::unicode::encoding::utf8::encode;
    const auto path = message.at<value::Str>(0);

    if (!interpreter.has_permission(Permission::Read))
    {
      return result_type::error({
        std::nullopt,
        U"Read access to file system required."
      });
    }

    return result_type::ok(interpreter.bool_value(
      !::chdir(encode(path->value()).c_str())
    ));
  }

  /**
   * Returns a string representing the current working directory. If the
   * current working directory is not available for some reason, null will be
   * returned instead.
   */
  static result_type
  func_cwd(Interpreter& interpreter, const Message&)
  {
    using peelo::unicode::encoding::utf8::decode_validate;
    char buffer[MAX_PATH];
    std::u32string result;

    if (!interpreter.has_permission(Permission::Read))
    {
      return result_type::error({
        std::nullopt,
        U"Read access to file system required."
      });
    }
    else if (!::getcwd(buffer, MAX_PATH) ||
      !decode_validate(buffer, std::strlen(buffer), result))
    {
      return result_type::ok(interpreter.null_value());
    }

    return result_type::ok(make_str(result));
  }

  /**
   * Changes owner of regular file or directory. Returns false if the operation
   * could not be performed for some reason, otherwise true.
   *
   *     chown("./file.txt", 1000, 1001)
   */
  static result_type
  func_chown(Interpreter& interpreter, const Message& message)
  {
    using peelo::unicode::encoding::utf8::encode;
    const auto path = message.at<value::Str>(0);
    const auto uid = message.at<value::Int>(1);
    const auto gid = message.at<value::Int>(2);

    if (!interpreter.has_permission(Permission::Write))
    {
      return result_type::error({
        std::nullopt,
        U"Write access to file system required."
      });
    }

    return result_type::ok(interpreter.bool_value(
      !::chown(
        encode(path->value()).c_str(),
        static_cast<::uid_t>(uid->value()),
        static_cast<::gid_t>(uid->value())
      )
    ));
  }

  // TODO: copy

  /**
   * Lists contents of an directory and returns them in an list of `DirEntry`
   * instances.
   */
  static result_type
  func_list(Interpreter& interpreter, const Message& message)
  {
    using peelo::unicode::encoding::utf8::decode;
    using peelo::unicode::encoding::utf8::encode;
    const auto path = message.at<value::Str>(0);
    DIR* dirp;
    value::List::container_type result;
    struct ::dirent* entity;

    if (!interpreter.has_permission(Permission::Read))
    {
      return result_type::error({
        std::nullopt,
        U"Read access to file system required."
      });
    }
    else if (!(dirp = ::opendir(encode(path->value()).c_str())))
    {
      return result_type::ok(interpreter.null_value());
    }
    while ((entity = ::readdir(dirp)))
    {
      // TODO: What to do with UTF-8 decoding errors?
      result.push_back(make_record({
        { U"name", make_str(decode(entity->d_name)) },
        { U"is_file", interpreter.bool_value(entity->d_type & DT_REG) },
        { U"is_dir", interpreter.bool_value(entity->d_type & DT_DIR) },
        { U"is_symlink", interpreter.bool_value(entity->d_type & DT_LNK) },
      }));
    }
    ::closedir(dirp);

    return result_type::ok(make_list(result));
  }

  // TODO: mkdir
  // TODO: read
  // TODO: real_path

  /**
   * Removes file from system. If the file cannot be removed for some reason,
   * false will be returned, otherwise true will be returned.
   *
   *     delete("./some-file.txt")
   */
  static result_type
  func_delete(Interpreter& interpreter, const Message& message)
  {
    using peelo::unicode::encoding::utf8::encode;
    const auto path = message.at<value::Str>(0);

    if (!interpreter.has_permission(Permission::Write))
    {
      return result_type::error({
        std::nullopt,
        U"Write access to file system required."
      });
    }

    return result_type::ok(interpreter.bool_value(
      !::unlink(encode(path->value()).c_str())
    ));
  }

  /**
   * Renames an file in the file system.
   *
   *     rename("./old-file.txt", "./new-file.txt")
   */
  static result_type
  func_rename(Interpreter& interpreter, const Message& message)
  {
    using peelo::unicode::encoding::utf8::encode;
    const auto old_path = message.at<value::Str>(0);
    const auto new_path = message.at<value::Str>(1);

    if (!interpreter.has_permission(Permission::Read))
    {
      return result_type::error({
        std::nullopt,
        U"Read access to file system required."
      });
    }
    else if (!interpreter.has_permission(Permission::Write))
    {
      return result_type::error({
        std::nullopt,
        U"Write access to file system required."
      });
    }

    return result_type::ok(interpreter.bool_value(
      !::rename(
        encode(old_path->value()).c_str(),
        encode(new_path->value()).c_str()
      )
    ));
  }

  // TODO: write

  // TODO: When we have binary types: read_binary, write_binary

  Scope
  create(const Interpreter& interpreter)
  {
    using namespace snek::type::utils;
    const auto& bool_type = interpreter.bool_type();
    const auto& int_type = interpreter.int_type();
    const auto& str_type = interpreter.str_type();
    const auto nullable_str_type = make_union_type({
      str_type,
      interpreter.void_type()
    });
    const auto direntry_type = make_record_type({
      { U"name", str_type },
      { U"is_file", bool_type },
      { U"is_dir", bool_type },
      { U"is_symlink", bool_type },
    });

    return create_module(
      {
        {
          U"chdir",
          func_chdir,
          { Parameter(U"path", str_type) },
          bool_type
        },
        {
          U"cwd",
          func_cwd,
          {},
          nullable_str_type
        },
        {
          U"chown",
          func_chown,
          {
            Parameter(U"path", str_type),
            Parameter(U"uid", int_type),
            Parameter(U"gid", int_type),
          },
          bool_type
        },
        {
          U"list",
          func_list,
          { Parameter(U"path", str_type) },
          bool_type
        },
        {
          U"delete",
          func_delete,
          { Parameter(U"path", str_type) },
          bool_type
        },
        {
          U"rename",
          func_rename,
          {
            Parameter(U"old_path", str_type),
            Parameter(U"new_path", str_type)
          },
          bool_type
        },
      },
      {
        { U"DirEntry", direntry_type },
      }
    );
  }
}
