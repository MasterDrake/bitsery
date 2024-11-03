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

#include <bitsery/ext/eastl_queue.h>

#include "serialization_test_utils.h"
#include <gmock/gmock.h>

void* __cdecl operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
	(void)name;
	(void)flags;
	(void)debugFlags;
	(void)file;
	(void)line;
	return new uint8_t[size];
}

void* __cdecl operator new[](size_t size, size_t alignement, size_t offset, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
	(void)name;
	(void)alignement;
	(void)offset;
	(void)flags;
	(void)debugFlags;
	(void)file;
	(void)line;
	return new uint8_t[size];
}

using EastlQueue = bitsery::ext::EastlQueue;

using testing::Eq;

// inherit from queue so we could take underlying container, because priority
// queue doesn't have equal operator defined
template<typename T, typename C>
struct PriorityQueueCnt : public eastl::priority_queue<T, C>
{
  static const C& getContainer(const eastl::priority_queue<T, C>& s)
  {
    // get address of underlying container
    return s.*(&PriorityQueueCnt::c);
  }
  static C& getContainer(eastl::priority_queue<T, C>& s)
  {
    // get address of underlying container
    return s.*(&PriorityQueueCnt::c);
  }
};

template<typename T>
void
test(SerializationContext& ctx, const T& v, T& r)
{
  ctx.createSerializer().ext4b(v, EastlQueue{ 10 });
  ctx.createDeserializer().ext4b(r, EastlQueue{ 10 });
}

TEST(SerializeExtensionEastlQueue, QueueDefaultContainer)
{
  eastl::queue<int32_t> t1{};
  t1.push(3);
  t1.push(-4854);
  eastl::queue<int32_t> r1{};

  SerializationContext ctx1{};
  test(ctx1, t1, r1);
  EXPECT_THAT(t1, Eq(r1));
}

TEST(SerializeExtensionEastlQueue, QueueVectorContainer)
{
  eastl::queue<int32_t, eastl::vector<int32_t>> t1{};
  t1.push(3);
  t1.push(-4854);
  eastl::queue<int32_t, eastl::vector<int32_t>> r1{};

  SerializationContext ctx1{};
  test(ctx1, t1, r1);
  EXPECT_THAT(t1, Eq(r1));
}

TEST(SerializeExtensionEastlQueue, PriorityQueueDefaultContainer)
{
  eastl::priority_queue<int32_t> t1{};
  t1.push(3);
  t1.push(-4854);
  eastl::priority_queue<int32_t> r1{};

  SerializationContext ctx1{};
  test(ctx1, t1, r1);
  auto& ct1 = PriorityQueueCnt<int32_t, eastl::vector<int32_t>>::getContainer(t1);
  auto& cr1 = PriorityQueueCnt<int32_t, eastl::vector<int32_t>>::getContainer(r1);
  EXPECT_THAT(ct1, Eq(cr1));
}

TEST(SerializeExtensionEastlQueue, PriorityQueueDequeContainer)
{
  eastl::priority_queue<int32_t, eastl::deque<int32_t>> t1{};
  t1.push(678);
  t1.push(-44);
  eastl::priority_queue<int32_t, eastl::deque<int32_t>> r1{};

  SerializationContext ctx1{};
  test(ctx1, t1, r1);
  auto& ct1 = PriorityQueueCnt<int32_t, eastl::deque<int32_t>>::getContainer(t1);
  auto& cr1 = PriorityQueueCnt<int32_t, eastl::deque<int32_t>>::getContainer(r1);
  EXPECT_THAT(ct1, Eq(cr1));
}
