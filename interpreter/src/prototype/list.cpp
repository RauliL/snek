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
#include "snek/error.hpp"
#include "snek/interpreter/runtime.hpp"

namespace snek::interpreter::prototype
{
  template<class T>
  static inline const T*
  As(const value::ptr& value)
  {
    return static_cast<const T*>(value.get());
  }

  /**
   * List#filter(this: List, callback: Function) => List
   *
   * Constructs new list which contains elements from this one for which the
   * given callback function has returned true for.
   */
  static value::ptr
  Filter(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    const auto list = As<value::List>(arguments[0]);
    const auto callback = As<value::Function>(arguments[1]);
    const auto size = list->GetSize();
    std::vector<value::ptr> result;

    for (std::size_t i = 0; i < size; ++i)
    {
      const auto element = list->At(i);

      if (value::ToBoolean(
        callback->Call(std::nullopt, runtime, { element }))
      )
      {
        result.push_back(element);
      }
    }

    return value::List::Make(result);
  }

  /**
   * List#forEach(this: List, callback: Function) => null
   *
   * Iterates the list and invokes given callback function with each list
   * element given as argument.
   */
  static value::ptr
  ForEach(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    const auto list = As<value::List>(arguments[0]);
    const auto callback = As<value::Function>(arguments[1]);
    const auto size = list->GetSize();

    for (std::size_t i = 0; i < size; ++i)
    {
      callback->Call(std::nullopt, runtime, { list->At(i) });
    }

    return nullptr;
  }

  /**
   * List#join(this: List, separator: String) => String
   *
   * Constructs an string where each element in the list is converted into
   * string and separated from each other with the given separator.
   */
  static value::ptr
  Join(Runtime&, const std::vector<value::ptr>& arguments)
  {
    const auto list = As<value::List>(arguments[0]);
    const auto& separator = As<value::String>(arguments[1])->value();
    const auto size = list->GetSize();
    std::u32string result;

    // TODO: Add support for overloading `toString`.
    for (std::size_t i = 0; i < size; ++i)
    {
      if (i > 0)
      {
        result.append(separator);
      }
      result.append(value::ToString(list->At(i)));
    }

    return std::make_shared<value::String>(result);
  }

  /**
   * List#map(this: List, callback: Function) => List
   *
   * Constructs new list from return values of given callback function when
   * called with element of this list as an argument.
   */
  static value::ptr
  Map(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    const auto list = As<value::List>(arguments[0]);
    const auto callback = As<value::Function>(arguments[1]);
    const auto size = list->GetSize();
    std::vector<value::ptr> result;

    result.reserve(size);
    for (std::size_t i = 0; i < size; ++i)
    {
      result.push_back(callback->Call(std::nullopt, runtime, { list->At(i) }));
    }

    return value::List::Make(result);
  }

  /**
   * List#reduce(this: List, callback: Function, initial: any = null) => any
   *
   * Executes given reducer function on each element in the list, passing the
   * return value of from the calculation on the preceding element. Optional
   * initial value can be given (it cannot be null) and if it's missing the
   * first element of the list is instead used as the initial value.
   */
  static value::ptr
  Reduce(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    const auto list = As<value::List>(arguments[0]);
    const auto callback = As<value::Function>(arguments[1]);
    const auto size = list->GetSize();
    value::ptr result;
    std::size_t start;

    if (!value::IsNull(arguments[2]))
    {
      result = arguments[2];
      start = 0;
    }
    else if (size > 0)
    {
      result = list->At(0);
      start = 1;
    } else {
      return nullptr;
    }
    for (std::size_t i = start; i < size; ++i)
    {
      result = callback->Call(std::nullopt, runtime, { result, list->At(i) });
    }

    return result;
  }

  /**
   * List#size(this: List) => Int
   *
   * Returns size of the list.
   */
  static value::ptr
  Size(Runtime&, const std::vector<value::ptr>& arguments)
  {
    return std::make_shared<value::Int>(As<value::List>(
      arguments[0]
    )->GetSize());
  }

  /**
   * List#[](this: List, index: Number) => any
   *
   * Returns list element from given index. If the index is out of bounds, an
   * exception will be thrown.
   */
  static value::ptr
  At(Runtime&, const std::vector<value::ptr>& arguments)
  {
    const auto list = As<value::List>(arguments[0]);
    const auto size = list->GetSize();
    auto index = std::static_pointer_cast<value::Number>(
      arguments[1]
    )->ToInt();

    if (index < 0)
    {
      index += size;
    }
    if (
      !size ||
      index < 0 ||
      index >= static_cast<value::Number::int_type>(size)
    )
    {
      throw Error{ std::nullopt, U"List index out of bounds." };
    }

    return list->At(index);
  }

  /**
   * List#+(this: List, other: List) => List
   *
   * Concatenates contents of two lists together.
   */
  static value::ptr
  Concat(Runtime&, const std::vector<value::ptr>& arguments)
  {
    return value::List::Concat(
      std::static_pointer_cast<value::List>(arguments[0]),
      std::static_pointer_cast<value::List>(arguments[1])
    );
  }

  void
  MakeList(const Runtime* runtime, value::Record::container_type& fields)
  {
    fields[U"filter"] = value::Function::MakeNative(
      {
        Parameter(U"this", runtime->list_type()),
        Parameter(
          U"callback",
          std::make_shared<type::Function>(
            std::vector<Parameter>{ Parameter(U"element") },
            runtime->boolean_type()
          )
        ),
      },
      runtime->list_type(),
      Filter
    );
    fields[U"forEach"] = value::Function::MakeNative(
      {
        Parameter(U"this", runtime->list_type()),
        Parameter(
          U"callback",
          std::make_shared<type::Function>(
            std::vector<Parameter>{ Parameter(U"element") },
            runtime->any_type()
          )
        ),
      },
      runtime->void_type(),
      ForEach
    );
    fields[U"join"] = value::Function::MakeNative(
      {
        Parameter(U"this", runtime->list_type() ),
        Parameter(U"separator", runtime->string_type())
      },
      runtime->string_type(),
      Join
    );
    fields[U"map"] = value::Function::MakeNative(
      {
        Parameter(U"this", runtime->list_type()),
        Parameter(
          U"callback",
          std::make_shared<type::Function>(
            std::vector<Parameter>{ Parameter(U"element") },
            runtime->any_type()
          )
        )
      },
      runtime->list_type(),
      Map
    );
    fields[U"reduce"] = value::Function::MakeNative(
      {
        Parameter(U"this", runtime->list_type()),
        Parameter(
          U"callback",
          std::make_shared<type::Function>(
            std::vector<Parameter>{
              Parameter(U"accumulator"),
              Parameter(U"current")
            },
            runtime->any_type()
          )
        ),
        Parameter(
          U"initial",
          runtime->any_type(),
          std::make_shared<parser::expression::Null>(Position{ U"", 0, 0 })
        )
      },
      runtime->list_type(),
      Reduce
    );
    fields[U"size"] = value::Function::MakeNative(
      {
        Parameter(U"this", runtime->list_type())
      },
      runtime->int_type(),
      Size
    );

    fields[U"[]"] = value::Function::MakeNative(
      {
        Parameter(U"this", runtime->list_type()),
        Parameter(U"index", runtime->number_type())
      },
      runtime->any_type(),
      At
    );
    fields[U"+"] = value::Function::MakeNative(
      {
        Parameter(U"this", runtime->list_type()),
        Parameter(U"index", runtime->list_type())
      },
      runtime->list_type(),
      Concat
    );
  }
}
