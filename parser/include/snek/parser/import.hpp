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

#include "snek/parser/lexer.hpp"

namespace snek::parser
{
  enum class ImportKind
  {
    Named,
    Star,
  };

  class Import : public TypedNode<ImportKind>
  {
  public:
    DISALLOW_COPY_AND_ASSIGN(Import);

    class Named;
    class Star;

    const std::optional<std::u32string> alias;

    explicit Import(
      const std::optional<Position>& position,
      ImportKind kind,
      const std::optional<std::u32string>& alias_ = std::nullopt
    )
      : TypedNode(position, kind)
      , alias(alias_) {}

    static import_ptr ParseSpecifier(Lexer& lexer);
  };

  class Import::Named final : public Import
  {
  public:
    const std::u32string name;

    explicit Named(
      const std::optional<Position>& position,
      const std::u32string name_,
      const std::optional<std::u32string>& alias = std::nullopt
    )
      : Import(position, ImportKind::Named, alias)
      , name(name_) {}

    inline std::u32string ToString() const override
    {
      auto result = name;

      if (alias)
      {
        result.append(U" as ").append(*alias);
      }

      return result;
    }
  };

  class Import::Star final : public Import
  {
  public:
    explicit Star(
      const std::optional<Position>& position,
      const std::optional<std::u32string>& alias = std::nullopt
    )
      : Import(position, ImportKind::Star, alias) {}

    inline std::u32string ToString() const override
    {
      std::u32string result(U"*");

      if (alias)
      {
        result.append(U" as ").append(*alias);
      }

      return result;
    }
  };
}
