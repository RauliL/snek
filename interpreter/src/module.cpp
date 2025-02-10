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
#include <fstream>

#include <peelo/unicode/encoding/utf8.hpp>

#include "snek/interpreter/error.hpp"
#include "snek/interpreter/runtime.hpp"

namespace snek::interpreter
{
  Scope::ptr
  ImportFilesystemModule(
    const std::optional<Position>& position,
    Runtime& runtime,
    const std::u32string& path
  )
  {
    using peelo::unicode::encoding::utf8::encode;

    std::ifstream ifs(encode(path));
    std::string source;
    Scope::ptr module;

    if (!ifs.good())
    {
      throw runtime.MakeError(U"Unable to find module `" + path + U"'.");
    }
    source.append(
      std::istreambuf_iterator<char>(ifs),
      std::istreambuf_iterator<char>()
    );
    ifs.close();
    module = std::make_shared<Scope>(runtime.root_scope());
    module->DeclareVariable(
      position,
      U"__name__",
      value::String::Make(path)
    );
    runtime.RunScript(module, source, path);

    return module;
  }
}
