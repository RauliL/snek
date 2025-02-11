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
#include "snek/interpreter/error.hpp"
#include "snek/interpreter/runtime.hpp"

namespace snek::interpreter::prototype
{
  template<class T>
  static inline const T*
  As(const value::ptr& value)
  {
    return static_cast<const T*>(value.get());
  }

  static value::List::size_type
  AsIndex(
    const Runtime& runtime,
    const value::List* list,
    const value::ptr& index_value
  )
  {
    const auto size = list->GetSize();
    auto index = static_cast<const value::Int*>(index_value.get())->value();

    if (index < 0)
    {
      index += size;
    }
    if (!size || size < 0 || index >= static_cast<std::int64_t>(size))
    {
      throw runtime.MakeError(U"List index out of bounds.");
    }

    return static_cast<value::List::size_type>(index);
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
    const auto callback = std::static_pointer_cast<value::Function>(
      arguments[1]
    );
    const auto size = list->GetSize();
    std::vector<value::ptr> result;

    for (std::size_t i = 0; i < size; ++i)
    {
      const auto element = list->At(i);

      if (value::ToBoolean(
        value::Function::Call(
          std::nullopt,
          runtime,
          callback,
          {
            element,
            runtime.MakeInt(static_cast<std::int64_t>(i)),
          }
        )
      ))
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
    const auto callback = std::static_pointer_cast<value::Function>(
      arguments[1]
    );
    const auto size = list->GetSize();

    for (std::size_t i = 0; i < size; ++i)
    {
      value::Function::Call(
        std::nullopt,
        runtime,
        callback,
        {
          list->At(i),
          runtime.MakeInt(static_cast<std::int64_t>(i)),
        }
      );
    }

    return nullptr;
  }

  /**
   * List#indexOf(this: List, element: any, start: Int = 0) => Int | null
   *
   * Returns index of given list of which element is equal with the given
   * value. If the value does not exist in the list, `null` is returned
   * instead.
   */
  static value::ptr
  IndexOf(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    const auto list = As<value::List>(arguments[0]);
    const auto element = arguments[1];
    const auto start = AsIndex(runtime, list, arguments[2]);
    const auto size = list->GetSize();

    for (std::size_t i = start; i < size; ++i)
    {
      // TODO: Add support for "==" method.
      if (value::Equals(list->At(i), element))
      {
        return runtime.MakeInt(static_cast<std::int64_t>(i));
      }
    }

    return nullptr;
  }

  /**
   * List#includes(this: List, element: any) => Boolean
   *
   * Returns `true` if given list contains given value, `false` otherwise.
   */
  static value::ptr
  Includes(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    const auto list = As<value::List>(arguments[0]);
    const auto element = arguments[1];
    const auto size = list->GetSize();

    for (std::size_t i = 0; i < size; ++i)
    {
      // TODO: Add support for "==" method.
      if (value::Equals(list->At(i), element))
      {
        return runtime.MakeBoolean(true);
      }
    }

    return runtime.MakeBoolean(false);
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
    const auto separator = As<value::String>(arguments[1])->ToString();
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

    return value::String::Make(result);
  }

  /**
   * List#lastIndexOf(
   *   this: List,
   *   element: any,
   *   start: Int | null = null,
   * ) => Int | null
   *
   * Returns last index of given list of which element is equal with the given
   * value. If the value does not exist in the list, `null` is returned
   * instead.
   */
  static value::ptr
  LastIndexOf(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    const auto list = As<value::List>(arguments[0]);
    const auto element = arguments[1];
    const auto size = list->GetSize();
    std::size_t start;

    if (arguments[2])
    {
      start = AsIndex(runtime, list, arguments[2]);
    } else {
      start = size - 1;
    }
    for (;;)
    {
      if (start >= size)
      {
        return nullptr;
      }
      else if (value::Equals(list->At(start), element))
      {
        return runtime.MakeInt(static_cast<std::int64_t>(start));
      }
      else if (!--start)
      {
        return nullptr;
      }
    }
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
    const auto callback = std::static_pointer_cast<value::Function>(
      arguments[1]
    );
    const auto size = list->GetSize();
    std::vector<value::ptr> result;

    result.reserve(size);
    for (std::size_t i = 0; i < size; ++i)
    {
      result.push_back(value::Function::Call(
        std::nullopt,
        runtime,
        callback,
        {
          list->At(i),
          runtime.MakeInt(static_cast<std::int64_t>(i)),
        }
      ));
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
    const auto callback = std::static_pointer_cast<value::Function>(
      arguments[1]
    );
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
      result = value::Function::Call(
        std::nullopt,
        runtime,
        callback,
        {
          result,
          list->At(i),
          runtime.MakeInt(static_cast<std::int64_t>(i)),
        }
      );
    }

    return result;
  }

  namespace
  {
    class ReverseList final : public value::List
    {
    public:
      explicit ReverseList(const std::shared_ptr<List>& list)
        : m_list(list) {}

      inline size_type GetSize() const override
      {
        return m_list->GetSize();
      }

      inline value_type At(size_type index) const override
      {
        return m_list->At(GetSize() - index - 1);
      }

    private:
      const std::shared_ptr<List> m_list;
    };
  }

  /**
   * List#reverse(this: List) => List
   *
   * Returns reversed copy of the list.
   */
  static value::ptr
  Reverse(Runtime&, const std::vector<value::ptr>& arguments)
  {
    return std::make_shared<ReverseList>(
      std::static_pointer_cast<value::List>(arguments[0])
    );
  }

  /**
   * List#size(this: List) => Int
   *
   * Returns size of the list.
   */
  static value::ptr
  Size(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    return runtime.MakeInt(As<value::List>(arguments[0])->GetSize());
  }

  /**
   * List#[](this: List, index: Number) => any
   *
   * Returns list element from given index. If the index is out of bounds, an
   * exception will be thrown.
   */
  static value::ptr
  At(Runtime& runtime, const std::vector<value::ptr>& arguments)
  {
    const auto list = As<value::List>(arguments[0]);

    return list->At(AsIndex(runtime, list, arguments[1]));
  }

  namespace
  {
    class ConcatList final : public value::List
    {
    public:
      explicit ConcatList(
        const std::shared_ptr<List>& left,
        const std::shared_ptr<List>& right
      )
        : m_left(left)
        , m_right(right) {}

      inline size_type GetSize() const override
      {
        return m_left->GetSize() + m_right->GetSize();
      }

      value_type At(size_type index) const override
      {
        const auto left_size = m_left->GetSize();

        if (index < left_size)
        {
          return m_left->At(index);
        } else {
          return m_right->At(index - left_size);
        }
      }

    private:
      const std::shared_ptr<List> m_left;
      const std::shared_ptr<List> m_right;
    };
  }

  /**
   * List#+(this: List, other: List) => List
   *
   * Concatenates contents of two lists together.
   */
  static value::ptr
  Concat(Runtime&, const std::vector<value::ptr>& arguments)
  {
    return std::make_shared<ConcatList>(
      std::static_pointer_cast<value::List>(arguments[0]),
      std::static_pointer_cast<value::List>(arguments[1])
    );
  }

  namespace
  {
    class RepeatList final : public value::List
    {
    public:
      explicit RepeatList(const std::shared_ptr<List>& list, size_type count)
        : m_list(list)
        , m_count(count)
        , m_size(list->GetSize()) {}

      inline size_type GetSize() const override
      {
        return m_count * m_size;
      }

      inline value_type At(size_type index) const override
      {
        while (index >= m_size)
        {
          index -= m_size;
        }

        return m_list->At(index);
      }

    private:
      const std::shared_ptr<List> m_list;
      const size_type m_count;
      const size_type m_size;
    };
  }

  /**
   * List(this: List, count: Int) => List
   *
   * Repeats list given number of times.
   */
  static value::ptr
  Repeat(Runtime&, const std::vector<value::ptr>& arguments)
  {
    const auto count = static_cast<std::size_t>(
      As<value::Int>(arguments[1])->value()
    );

    if (count == 1)
    {
      return arguments[0];
    }

    return std::make_shared<RepeatList>(
      std::static_pointer_cast<value::List>(arguments[0]),
      count
    );
  }

  void
  MakeList(const Runtime* runtime, value::Record::container_type& fields)
  {
    const auto optional_int = type::MakeOptional(runtime->int_type());

    fields[U"filter"] = value::Function::MakeNative(
      {
        { U"this", runtime->list_type() },
        {
          U"callback",
          std::make_shared<type::Function>(
            std::vector<Parameter>{
              { U"element" },
              { U"index", runtime->int_type() },
            },
            runtime->boolean_type()
          )
        },
      },
      runtime->list_type(),
      Filter
    );
    fields[U"forEach"] = value::Function::MakeNative(
      {
        { U"this", runtime->list_type() },
        {
          U"callback",
          std::make_shared<type::Function>(
            std::vector<Parameter>{
              { U"element" },
              { U"index", runtime->int_type() },
            },
            runtime->any_type()
          )
        },
      },
      runtime->void_type(),
      ForEach
    );
    fields[U"indexOf"] = value::Function::MakeNative(
      {
        { U"this", runtime->list_type() },
        { U"element" },
        {
          U"start",
          runtime->int_type(),
          std::make_shared<parser::expression::Int>(std::nullopt, 0)
        },
      },
      type::MakeOptional(runtime->int_type()),
      IndexOf
    );
    fields[U"includes"] = value::Function::MakeNative(
      {
        { U"this", runtime->list_type() },
        { U"element" },
      },
      runtime->boolean_type(),
      Includes
    );
    fields[U"join"] = value::Function::MakeNative(
      {
        { U"this", runtime->list_type() },
        { U"separator", runtime->string_type() },
      },
      runtime->string_type(),
      Join
    );
    fields[U"lastIndexOf"] = value::Function::MakeNative(
      {
        { U"this", runtime->list_type() },
        { U"element" },
        {
          U"start",
          optional_int,
          std::make_shared<parser::expression::Null>()
        },
      },
      type::MakeOptional(runtime->int_type()),
      LastIndexOf
    );
    fields[U"map"] = value::Function::MakeNative(
      {
        { U"this", runtime->list_type() },
        {
          U"callback",
          std::make_shared<type::Function>(
            std::vector<Parameter>{
              { U"element" },
              { U"index", runtime->int_type() },
            },
            runtime->any_type()
          )
        },
      },
      runtime->list_type(),
      Map
    );
    fields[U"reduce"] = value::Function::MakeNative(
      {
        { U"this", runtime->list_type() },
        {
          U"callback",
          std::make_shared<type::Function>(
            std::vector<Parameter>{
              { U"accumulator" },
              { U"current" },
              { U"index", runtime->int_type() },
            },
            runtime->any_type()
          )
        },
        {
          U"initial",
          runtime->any_type(),
          std::make_shared<parser::expression::Null>(std::nullopt)
        },
      },
      runtime->list_type(),
      Reduce
    );
    fields[U"reverse"] = value::Function::MakeNative(
      {
        { U"this", runtime->list_type() },
      },
      runtime->list_type(),
      Reverse
    );
    fields[U"size"] = value::Function::MakeNative(
      {
        { U"this", runtime->list_type() },
      },
      runtime->int_type(),
      Size
    );

    fields[U"[]"] = value::Function::MakeNative(
      {
        { U"this", runtime->list_type() },
        { U"index", runtime->number_type() },
      },
      runtime->any_type(),
      At
    );
    fields[U"+"] = value::Function::MakeNative(
      {
        { U"this", runtime->list_type() },
        { U"other", runtime->list_type() },
      },
      runtime->list_type(),
      Concat
    );
    fields[U"*"] = value::Function::MakeNative(
      {
        { U"this", runtime->list_type() },
        { U"count", runtime->int_type() },
      },
      runtime->list_type(),
      Repeat
    );
  }
}
