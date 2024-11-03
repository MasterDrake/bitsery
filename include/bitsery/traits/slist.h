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

#ifndef BITSERY_TRAITS_EASTL_SLIST_H
#define BITSERY_TRAITS_EASTL_SLIST_H

#include "../details/serialization_common.h"
#include "core/traits.h"
#include <EASTL/slist.h>

namespace bitsery {

namespace traits {

template<typename T, typename Allocator>
struct ContainerTraits<eastl::slist<T, Allocator>>
{
  using TValue = T;
  static constexpr bool isResizable = true;
  static constexpr bool isContiguous = false;
  static size_t size(const eastl::slist<T, Allocator>& container)
  {
    return static_cast<size_t>(
      eastl::distance(container.begin(), container.end()));
  }
  static void resize(eastl::slist<T, Allocator>& container, size_t size)
  {
    resizeImpl(container, size, eastl::is_default_constructible<TValue>{});
  }

private:
  using diff_t = typename eastl::slist<T, Allocator>::difference_type;
  static void resizeImpl(eastl::slist<T, Allocator>& container,
                         size_t size,
                         eastl::true_type)
  {
    container.resize(size);
  }
  static void resizeImpl(eastl::slist<T, Allocator>& container,
                         size_t newSize,
                         eastl::false_type)
  {
    const auto oldSize = size(container);
    for (auto it = oldSize; it < newSize; ++it) {
      container.push_front(::bitsery::Access::create<TValue>());
    }
    if (oldSize > newSize) {
      // erase_after must have atleast one element to work
      if (newSize > 0)
        container.erase_after(
          eastl::next(eastl::begin(container), static_cast<diff_t>(newSize - 1)));
      else
        container.clear();
    }
  }
};
}
}

#endif // BITSERY_TRAITS_EASTL_SLIST_H
