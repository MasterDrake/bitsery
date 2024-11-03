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

#include <bitsery/traits/array.h>
#include <bitsery/traits/deque.h>
#include <bitsery/traits/slist.h>
#include <bitsery/traits/list.h>

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

using testing::ContainerEq;
using testing::Eq;

/*
 * overload to get container of types
 */

template<typename Container>
Container
getFilledContainer()
{
  return { 1, 2, 3, 4, 5, 78, 456, 8, 54 };
}

template<>
eastl::vector<MyStruct1>
getFilledContainer<eastl::vector<MyStruct1>>()
{
  return { { 0, 1 }, { 2, 3 },   { 4, 5 },      { 6, 7 },
           { 8, 9 }, { 11, 34 }, { 5134, 1532 } };
}

template<>
eastl::list<MyStruct2>
getFilledContainer<eastl::list<MyStruct2>>()
{
  return { { MyStruct2::V1, { 0, 1 } }, { MyStruct2::V3, { -45, 45 } } };
}

struct EmptyFtor
{
  template<typename S, typename T>
  void operator()(S&, T&)
  {
  }
};

/*
 * start testing session
 */

template<typename T>
class SerializeContainerDynamicSizeArthmeticTypes : public testing::Test
{
public:
  using TContainer = T;
  using TValue = typename T::value_type;

  const TContainer src = getFilledContainer<TContainer>();
  TContainer res{};

  size_t getExpectedBufSize(const SerializationContext& ctx) const
  {
    auto size = bitsery::traits::ContainerTraits<TContainer>::size(src);
    return ctx.containerSizeSerializedBytesCount(size) + size * sizeof(TValue);
  }
};
// eastl::slist is not supported, because it doesn't have size() method
using SequenceContainersWithArthmeticTypes =
  ::testing::Types<eastl::vector<int>,
                   eastl::list<float>,
                   eastl::slist<int>,
                   eastl::deque<unsigned short>>;

TYPED_TEST_SUITE(SerializeContainerDynamicSizeArthmeticTypes,
                 SequenceContainersWithArthmeticTypes, );

TYPED_TEST(SerializeContainerDynamicSizeArthmeticTypes, Values)
{
  SerializationContext ctx{};
  using TValue = typename TestFixture::TValue;

  ctx.createSerializer().container<sizeof(TValue)>(this->src, 1000);
  ctx.createDeserializer().container<sizeof(TValue)>(this->res, 1000);

  EXPECT_THAT(ctx.getBufferSize(), Eq(this->getExpectedBufSize(ctx)));
  EXPECT_THAT(this->res, ContainerEq(this->src));
}

TYPED_TEST(SerializeContainerDynamicSizeArthmeticTypes,
           CustomFunctionIncrements)
{
  SerializationContext ctx{};
  using TValue = typename TestFixture::TValue;

  auto& ser = ctx.createSerializer();
  ser.container(this->src, 1000, [](decltype(ser)& ser, TValue& v) {
    ser.template value<sizeof(v)>(v);
  });
  auto& des = ctx.createDeserializer();
  des.container(this->res, 1000, [](decltype(des)& des, TValue& v) {
    des.template value<sizeof(v)>(v);
    // increment by 1 after reading
    v++;
  });
  // decrement result by 1, before comparing for eq
  for (auto& v : this->res)
    v = static_cast<TValue>(v - 1);

  EXPECT_THAT(ctx.getBufferSize(), Eq(this->getExpectedBufSize(ctx)));
  EXPECT_THAT(this->res, ContainerEq(this->src));
}

template<typename T>
class SerializeContainerDynamicSizeCompositeTypes : public testing::Test
{
public:
  using TContainer = T;
  using TValue = typename T::value_type;

  const TContainer src = getFilledContainer<TContainer>();
  TContainer res{};

  size_t getExpectedBufSize(const SerializationContext& ctx) const
  {
    return ctx.containerSizeSerializedBytesCount(src.size()) +
           src.size() * TValue::SIZE;
  }
};

using SerializeContainerDynamicSizeWithCompositeTypes =
  ::testing::Types<eastl::vector<MyStruct1>, eastl::list<MyStruct2>>;

TYPED_TEST_SUITE(SerializeContainerDynamicSizeCompositeTypes,
                 SerializeContainerDynamicSizeWithCompositeTypes, );

TYPED_TEST(SerializeContainerDynamicSizeCompositeTypes,
           DefaultSerializeFunction)
{
  SerializationContext ctx{};

  ctx.createSerializer().container(this->src, 1000);
  ctx.createDeserializer().container(this->res, 1000);

  EXPECT_THAT(ctx.getBufferSize(), Eq(this->getExpectedBufSize(ctx)));
  EXPECT_THAT(this->res, ContainerEq(this->src));
}

TYPED_TEST(SerializeContainerDynamicSizeCompositeTypes,
           CustomFunctionThatDoNothing)
{
  SerializationContext ctx{};

  ctx.createSerializer().container(this->src, 1000, EmptyFtor{});
  ctx.createDeserializer().container(this->res, 1000, EmptyFtor{});

  EXPECT_THAT(ctx.getBufferSize(),
              Eq(ctx.containerSizeSerializedBytesCount(this->src.size())));
}

template<typename T>
class SerializeContainerFixedSizeArithmeticTypes : public testing::Test
{
public:
  using TContainer = T;

  size_t getContainerSize()
  {
    T tmp{};
    return static_cast<size_t>(eastl::distance(eastl::begin(tmp), eastl::end(tmp)));
  }
};

using StaticContainersWithIntegralTypes =
  ::testing::Types<eastl::array<int16_t, 4>, int16_t[4]>;

TYPED_TEST_SUITE(SerializeContainerFixedSizeArithmeticTypes,
                 StaticContainersWithIntegralTypes, );

TYPED_TEST(SerializeContainerFixedSizeArithmeticTypes, ArithmeticValues)
{
  using Container = typename TestFixture::TContainer;
  Container src{ 5, 9, 15, -459 };
  Container res{};

  SerializationContext ctx;
  ctx.createSerializer().container<2>(src);
  ctx.createDeserializer().container<2>(res);

  EXPECT_THAT(ctx.getBufferSize(), Eq(this->getContainerSize() * 2));
  EXPECT_THAT(res, ContainerEq(src));
}

template<typename T>
class SerializeContainerFixedSizeCompositeTypes
  : public SerializeContainerFixedSizeArithmeticTypes<T>
{
};

using StaticContainersWithCompositeTypes =
  ::testing::Types<eastl::array<MyStruct1, 4>, MyStruct1[4]>;

TYPED_TEST_SUITE(SerializeContainerFixedSizeCompositeTypes,
                 StaticContainersWithCompositeTypes, );

TYPED_TEST(SerializeContainerFixedSizeCompositeTypes,
           DefaultSerializationFunction)
{
  using Container = typename TestFixture::TContainer;
  Container src{ MyStruct1{ 0, 1 },
                 MyStruct1{ 8, 9 },
                 MyStruct1{ 11, 34 },
                 MyStruct1{ 5134, 1532 } };
  Container res{};

  SerializationContext ctx{};
  ctx.createSerializer().container(src);
  ctx.createDeserializer().container(res);

  EXPECT_THAT(ctx.getBufferSize(),
              Eq(this->getContainerSize() * MyStruct1::SIZE));
  EXPECT_THAT(res, ContainerEq(src));
}

TYPED_TEST(SerializeContainerFixedSizeCompositeTypes,
           CustomFunctionThatSerializesAnEmptyByteEveryElement)
{
  using Container = typename TestFixture::TContainer;
  Container src{ MyStruct1{ 0, 1 },
                 MyStruct1{ 2, 3 },
                 MyStruct1{ 4, 5 },
                 MyStruct1{ 5134, 1532 } };
  Container res{};

  using TValue = decltype(*eastl::begin(res));

  SerializationContext ctx{};
  auto& ser = ctx.createSerializer();
  ser.container(src, [](decltype(ser)& ser, TValue& v) {
    char tmp{};
    ser.object(v);
    ser.value1b(tmp);
  });
  auto& des = ctx.createDeserializer();
  des.container(res, [](decltype(des)& des, TValue& v) {
    char tmp{};
    des.object(v);
    des.value1b(tmp);
  });

  EXPECT_THAT(ctx.getBufferSize(),
              Eq(this->getContainerSize() * (MyStruct1::SIZE + sizeof(char))));
  EXPECT_THAT(res, ContainerEq(src));
}

class SerializeContainer : public ::testing::TestWithParam<size_t>
{};

TEST_P(SerializeContainer, SizeHasVariableLength)
{
  SerializationContext ctx{};

  eastl::vector<uint8_t> src(GetParam());
  eastl::vector<uint8_t> res{};
  ctx.createSerializer().container(
    src, eastl::numeric_limits<size_t>::max(), EmptyFtor{});
  ctx.createDeserializer().container(
    res, eastl::numeric_limits<size_t>::max(), EmptyFtor{});

  EXPECT_THAT(res.size(), Eq(src.size()));
  EXPECT_THAT(ctx.getBufferSize(),
              Eq(ctx.containerSizeSerializedBytesCount(src.size())));
}

INSTANTIATE_TEST_SUITE_P(LargeContainerSize,
                         SerializeContainer,
                         ::testing::Values(0x01, 0x80, 0x4000));
