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

#include <memory>

namespace snek { class Interpreter; }
namespace snek::type { class Base; }

namespace snek::value
{
  enum class Kind
  {
    Bool,
    Float,
    Func,
    Int,
    List,
    Null,
    Record,
    Str,
  };

  /**
   * Abstract base class for all values.
   */
  class Base
  {
  public:
    explicit Base();

    /**
     * Returns the type of the value.
     */
    virtual Kind kind() const = 0;

    /**
     * Determines type of the value.
     */
    virtual std::shared_ptr<type::Base> type(
      const Interpreter& interpreter
    ) const = 0;

    virtual bool equals(const std::shared_ptr<Base>& that) const = 0;

    virtual std::u32string to_string() const = 0;

    Base(const Base&) = delete;
    Base(Base&&) = delete;
    void operator=(const Base&) = delete;
    void operator=(Base&&) = delete;
  };

  using Ptr = std::shared_ptr<Base>;

  inline bool
  operator==(const Ptr& a, const Ptr& b)
  {
    return a->equals(b);
  }

  inline bool
  operator!=(const Ptr& a, const Ptr& b)
  {
    return !a->equals(b);
  }
}
