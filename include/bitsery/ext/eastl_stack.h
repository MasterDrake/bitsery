// MIT License
//
// Copyright (c) 2017 Mindaugas Vinkelis
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef BITSERY_EXT_EASTL_STACK_H
#define BITSERY_EXT_EASTL_STACK_H

#include "../traits/deque.h"
#include <EASTL/stack.h>

namespace bitsery {
namespace ext {

class EastlStack
{
private:
  // inherit from stack so we could take underlying container
  template<typename T, typename C>
  struct StackCnt : public eastl::stack<T, C>
  {
    static const C& getContainer(const eastl::stack<T, C>& s)
    {
      // get address of underlying container
      return s.*(&StackCnt::c);
    }
    static C& getContainer(eastl::stack<T, C>& s)
    {
      // get address of underlying container
      return s.*(&StackCnt::c);
    }
  };
  size_t _maxSize;

public:
  explicit EastlStack(size_t maxSize)
    : _maxSize{ maxSize } {};

  template<typename Ser, typename T, typename C, typename Fnc>
  void serialize(Ser& ser, const eastl::stack<T, C>& obj, Fnc&& fnc) const
  {
    ser.container(
      StackCnt<T, C>::getContainer(obj), _maxSize, eastl::forward<Fnc>(fnc));
  }

  template<typename Des, typename T, typename C, typename Fnc>
  void deserialize(Des& des, eastl::stack<T, C>& obj, Fnc&& fnc) const
  {
    des.container(
      StackCnt<T, C>::getContainer(obj), _maxSize, eastl::forward<Fnc>(fnc));
  }
};
}

namespace traits {
template<typename T, typename Seq>
struct ExtensionTraits<ext::EastlStack, eastl::stack<T, Seq>>
{
  using TValue = T;
  static constexpr bool SupportValueOverload = true;
  static constexpr bool SupportObjectOverload = true;
  static constexpr bool SupportLambdaOverload = true;
};
}

}

#endif // BITSERY_EXT_EASTL_STACK_H
