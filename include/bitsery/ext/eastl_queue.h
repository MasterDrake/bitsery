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

#ifndef BITSERY_EXT_EASTL_QUEUE_H
#define BITSERY_EXT_EASTL_QUEUE_H

// include type traits for deque and vector, because they are defaults for queue and priority_queue
#include "../traits/deque.h"
#include "../traits/vector.h"
#include <EASTL/queue.h>
#include <EASTL/priority_queue.h>
#include <EASTL/type_traits.h>

namespace bitsery {
namespace ext {

class EastlQueue
{
private:
  // inherit from queue so we could take underlying container
  template<typename T, typename C>
  struct QueueCnt : public eastl::queue<T, C>
  {
    static const C& getContainer(const eastl::queue<T, C>& s)
    {
      // get address of underlying container
      return s.*(&QueueCnt::c);
    }
    static C& getContainer(eastl::queue<T, C>& s)
    {
      // get address of underlying container
      return s.*(&QueueCnt::c);
    }
  };
  // inherit from queue so we could take underlying container
  template<typename T, typename Seq, typename Cmp>
  struct PriorityQueueCnt : public eastl::priority_queue<T, Seq, Cmp>
  {
    static const Seq& getContainer(const eastl::priority_queue<T, Seq, Cmp>& s)
    {
      // get address of underlying container
      return s.*(&PriorityQueueCnt::c);
    }
    static Seq& getContainer(eastl::priority_queue<T, Seq, Cmp>& s)
    {
      // get address of underlying container
      return s.*(&PriorityQueueCnt::c);
    }
  };

  size_t _maxSize;

public:
  explicit EastlQueue(size_t maxSize)
    : _maxSize{ maxSize } {};

  // for queue
  template<typename Ser, typename T, typename C, typename Fnc>
  void serialize(Ser& ser, const eastl::queue<T, C>& obj, Fnc&& fnc) const
  {
    ser.container(
      QueueCnt<T, C>::getContainer(obj), _maxSize, eastl::forward<Fnc>(fnc));
  }

  template<typename Des, typename T, typename C, typename Fnc>
  void deserialize(Des& des, eastl::queue<T, C>& obj, Fnc&& fnc) const
  {
    des.container(
      QueueCnt<T, C>::getContainer(obj), _maxSize, eastl::forward<Fnc>(fnc));
  }

  // for priority_queue
  template<typename Ser, typename T, typename C, typename Comp, typename Fnc>
  void serialize(Ser& ser,
                 const eastl::priority_queue<T, C, Comp>& obj,
                 Fnc&& fnc) const
  {
    ser.container(PriorityQueueCnt<T, C, Comp>::getContainer(obj),
                  _maxSize,
                  eastl::forward<Fnc>(fnc));
  }

  template<typename Des, typename T, typename C, typename Comp, typename Fnc>
  void deserialize(Des& des,
                   eastl::priority_queue<T, C, Comp>& obj,
                   Fnc&& fnc) const
  {
    des.container(PriorityQueueCnt<T, C, Comp>::getContainer(obj),
                  _maxSize,
                  eastl::forward<Fnc>(fnc));
  }
};
}

namespace traits {
template<typename T>
struct ExtensionTraits<ext::EastlQueue, T>
{
  using TValue = typename T::value_type;
  static constexpr bool SupportValueOverload = true;
  static constexpr bool SupportObjectOverload = true;
  static constexpr bool SupportLambdaOverload = true;
};
}

}

#endif // BITSERY_EXT_EASTL_QUEUE_H
