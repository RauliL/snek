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
#include <snek/interpreter.hpp>
#include <snek/message.hpp>
#include <snek/value/utils.hpp>
#include <snek/type/utils.hpp>

namespace snek::api::list
{
  using namespace snek::value::utils;
  using result_type = value::Func::result_type;

  /**
   * Determines whether an list is empty.
   *
   *     isEmpty([]) == true
   *     isEmpty([1, 2, 3]) == false
   */
  static result_type
  func_isEmpty(Interpreter& interpreter, const Message& message)
  {
    const auto& input = message.at<value::List>(0);

    return result_type::ok(interpreter.bool_value(input->elements().empty()));
  }

  /**
   * Returns the number of elements in an list.
   *
   *     length([1, 2, 3]) == 3
   */
  static result_type
  func_length(Interpreter& interpreter, const Message& message)
  {
    const auto& input = message.at<value::List>(0);

    return result_type::ok(make_int(input->elements().size()));
  }

  /**
   * Reverses an string.
   *
   *     reverse([1, 2, 3]) == [3, 2, 1]
   */
  static result_type
  func_reverse(Interpreter& interpreter, const Message& message)
  {
    const auto& input = message.at<value::List>(0);

    return result_type::ok(make_list(input->rbegin(), input->rend()));
  }

  /**
   * Determines whether an list contains a certain value among it's elements.
   *
   *     includes([1, 2], 1) == true
   *     includes([1, 2], 3) == false
   */
  static result_type
  func_includes(Interpreter& interpreter, const Message& message)
  {
    const auto& input = message.at<value::List>(0);
    const auto& searched_element = message.at<value::Base>(1);

    for (const auto& element : *input)
    {
      if (element->equals(searched_element))
      {
        return result_type::ok(interpreter.true_value());
      }
    }

    return result_type::ok(interpreter.false_value());
  }

  /**
   * Executes provided function once for each element in the list and finally
   * returns the list.
   *
   *     forEach([1, 2], (n: Int): print(n)) == [1, 2]
   */
  static result_type
  func_forEach(Interpreter& interpreter, const Message& message)
  {
    const auto& input = message.at<value::List>(0);
    const auto& func = message.at<value::Func>(1);

    for (const auto& element : *input)
    {
      const auto result = func->send(interpreter, { element });

      if (!result)
      {
        return result;
      }
    }

    return result_type::ok(input);
  }

  /**
   * Creates a new list with all elements that pass the test implemented by
   * given function.
   *
   *     filter([1, 2, 3, 4], (n: Int) => n > 2) == [3, 4]
   */
  static result_type
  func_filter(Interpreter& interpreter, const Message& message)
  {
    const auto& input = message.at<value::List>(0);
    const auto& func = message.at<value::Func>(1);
    value::List::container_type output;

    for (const auto& element : *input)
    {
      const auto result = func->send(interpreter, { element });

      if (!result)
      {
        return result;
      }
      else if (result.value()->kind() != value::Kind::Bool)
      {
        return result_type::error({
          std::nullopt,
          U"Expected Bool, got " +
          result.value()->type(interpreter)->to_string() +
          U" instead."
        });
      }
      else if (std::static_pointer_cast<value::Bool>(result.value())->value())
      {
        output.push_back(element);
      }
    }

    return result_type::ok(make_list(output));
  }

  /**
   * Creates a new list populated with results of calling the provided function
   * on every element in the list.
   *
   *     map([1, 2], (n: Int) => n * 2) == [2, 4]
   */
  static result_type
  func_map(Interpreter& interpreter, const Message& message)
  {
    const auto& input = message.at<value::List>(0);
    const auto& func = message.at<value::Func>(1);
    value::List::container_type output;

    for (const auto& element : *input)
    {
      const auto result = func->send(interpreter, { element });

      if (!result)
      {
        return result;
      }
      output.push_back(result.value());
    }

    return result_type::ok(make_list(output));
  }

  Scope
  create(const Interpreter& interpreter)
  {
    using namespace snek::type::utils;
    const auto& any_type = interpreter.any_type();
    const auto& bool_type = interpreter.bool_type();
    const auto list_type = make_list_type(any_type);
    const auto func_type = make_func_type(
      { Parameter(U"element", any_type) },
      any_type
    );

    return Scope(
      {},
      {
        {
          U"isEmpty",
          {
            make_func(
              { Parameter(U"input", list_type) },
              func_isEmpty,
              bool_type
            ),
            true
          }
        },
        {
          U"length",
          {
            make_func(
              { Parameter(U"input", list_type) },
              func_length,
              interpreter.int_type()
            ),
            true
          }
        },
        {
          U"reverse",
          {
            make_func(
              { Parameter(U"input", list_type) },
              func_reverse,
              list_type
            ),
            true
          }
        },
        {
          U"includes",
          {
            make_func(
              {
                Parameter(U"input", list_type),
                Parameter(U"element", any_type),
              },
              func_includes,
              bool_type
            ),
            true
          }
        },
        {
          U"forEach",
          {
            make_func(
              {
                Parameter(U"input", list_type),
                Parameter(U"func", func_type),
              },
              func_forEach,
              interpreter.void_type()
            ),
            true
          }
        },
        {
          U"filter",
          {
            make_func(
              {
                Parameter(U"input", list_type),
                Parameter(
                  U"func",
                  make_func_type(
                    { Parameter(U"element", any_type) },
                    bool_type
                  )
                ),
              },
              func_filter,
              // TODO: See if we could implement some kind of generics at some
              // point.
              any_type
            ),
            true
          },
        },
        {
          U"map",
          {
            make_func(
              {
                Parameter(U"input", list_type),
                Parameter(U"func", func_type),
              },
              func_map,
              // TODO: See if we could implement some kind of generics at some
              // point.
              any_type
            ),
            true
          }
        },
      }
    );
  }
}

