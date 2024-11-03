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

#ifndef BITSERY_BRIEF_SYNTAX_TYPE_EASTL_UNORDERED_SET_H
#define BITSERY_BRIEF_SYNTAX_TYPE_EASTL_UNORDERED_SET_H

#include "../ext/eastl_set.h"
#include <EASTL/numeric_limits.h>
#include <EASTL/unordered_set.h>

namespace bitsery {
template<typename S,
         typename Key,
         typename Hash,
         typename KeyEqual,
         typename Allocator>
void
serialize(S& s,
          eastl::unordered_set<Key, Hash, KeyEqual, Allocator>& obj,
          size_t maxSize = eastl::numeric_limits<size_t>::max())
{
  s.ext(obj, ext::EastlSet{ maxSize });
}

template<typename S,
         typename Key,
         typename Hash,
         typename KeyEqual,
         typename Allocator>
void
serialize(S& s,
          eastl::unordered_multiset<Key, Hash, KeyEqual, Allocator>& obj,
          size_t maxSize = eastl::numeric_limits<size_t>::max())
{
  s.ext(obj, ext::EastlSet{ maxSize });
}

}

#endif // BITSERY_BRIEF_SYNTAX_TYPE_EASTL_UNORDERED_SET_H
