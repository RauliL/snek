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
#include <optional>
#include <type_traits>

#include "snek/position.hpp"

namespace snek::parser
{
  /**
   * Representation of an AST node.
   */
  class Node
  {
  public:
    /** Position of the node in source code, if known. */
    std::optional<Position> position;

    Node(const std::optional<Position>& position_ = std::nullopt)
      : position(position_) {}

    /**
     * Returns human readable desription of the node.
     */
    virtual std::u32string ToString() const = 0;
  };

  /**
   * AST node that has type information.
   */
  template<
    typename NodeType,
    typename = typename std::enable_if<std::is_enum<NodeType>::value>::type
  >
  class TypedNode : public Node
  {
  public:
    const NodeType kind;

    explicit TypedNode(
      const std::optional<Position>& position,
      NodeType kind_
    )
      : Node(position)
      , kind(kind_) {}
  };

  namespace element
  {
    class Base;
    using ptr = std::shared_ptr<Base>;
  }

  namespace expression
  {
    class Base;
    using ptr = std::shared_ptr<Base>;
  }

  namespace field
  {
    class Base;
    using ptr = std::shared_ptr<Base>;
  }

  namespace import
  {
    class Base;
    using ptr = std::shared_ptr<Base>;
  }

  namespace statement
  {
    class Base;
    using ptr = std::shared_ptr<Base>;
  }

  namespace type
  {
    class Base;
    using ptr = std::shared_ptr<Base>;
  }
}
