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

#include "serialization_test_utils.h"
#include <gmock/gmock.h>

#include <bitsery/ext/eastl_optional.h>
#include <bitsery/ext/value_range.h>

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

using EastlOptional = bitsery::ext::EastlOptional;

using BPSer = SerializationContext::TSerializer::BPEnabledType;
using BPDes = SerializationContext::TDeserializer::BPEnabledType;

using testing::Eq;

template<typename T>
void
test(SerializationContext& ctx, const T& v, T& r)
{
  ctx.createSerializer().ext4b(v, EastlOptional{});
  ctx.createDeserializer().ext4b(r, EastlOptional{});
}

TEST(SerializeExtensionEastlOptional, EmptyOptional)
{
  eastl::optional<int32_t> t1{};
  eastl::optional<int32_t> r1{};

  SerializationContext ctx1;
  test(ctx1, t1, r1);
  EXPECT_THAT(ctx1.getBufferSize(), Eq(1));
  EXPECT_THAT(t1, Eq(r1));

  r1 = 3;
  SerializationContext ctx2;
  test(ctx2, t1, r1);
  EXPECT_THAT(ctx2.getBufferSize(), Eq(1));
  EXPECT_THAT(t1, Eq(r1));
}

TEST(SerializeExtensionEastlOptional, OptionalHasValue)
{
  eastl::optional<int32_t> t1{ 43 };
  eastl::optional<int32_t> r1{ 52 };

  SerializationContext ctx1;
  test(ctx1, t1, r1);
  EXPECT_THAT(ctx1.getBufferSize(), Eq(1 + sizeof(int)));
  EXPECT_THAT(t1.value(), Eq(r1.value()));

  r1 = eastl::optional<int>{};
  SerializationContext ctx2;
  test(ctx2, t1, r1);
  EXPECT_THAT(ctx2.getBufferSize(), Eq(1 + sizeof(int)));
  EXPECT_THAT(t1.value(), Eq(r1.value()));
}

TEST(SerializeExtensionEastlOptional, AlignAfterStateWriteRead)
{
  eastl::optional<int32_t> t1{ 43 };
  eastl::optional<int32_t> r1{ 52 };
  auto range = bitsery::ext::ValueRange<int>{ 40, 60 };

  SerializationContext ctx;
  ctx.createSerializer().enableBitPacking([&t1, &range](BPSer& ser) {
    ser.ext(t1, EastlOptional(true), [&range](BPSer& ser, int32_t& v) {
      ser.ext(v, range);
    });
  });
  ctx.createDeserializer().enableBitPacking([&r1, &range](BPDes& des) {
    des.ext(r1, EastlOptional(true), [&range](BPDes& des, int32_t& v) {
      des.ext(v, range);
    });
  });

  EXPECT_THAT(ctx.getBufferSize(), Eq(2)); // 1byte for index + 1byte for value
  EXPECT_THAT(t1.value(), Eq(r1.value()));
}

TEST(SerializeExtensionEastlOptional, NoAlignAfterStateWriteRead)
{
  eastl::optional<int32_t> t1{ 43 };
  eastl::optional<int32_t> r1{ 52 };
  auto range = bitsery::ext::ValueRange<int>{ 40, 60 };

  SerializationContext ctx;
  ctx.createSerializer().enableBitPacking([&t1, &range](BPSer& ser) {
    ser.ext(t1, EastlOptional(false), [&range](BPSer& ser, int32_t& v) {
      ser.ext(v, range);
    });
  });
  ctx.createDeserializer().enableBitPacking([&r1, &range](BPDes& des) {
    des.ext(r1, EastlOptional(false), [&range](BPDes& des, int32_t& v) {
      des.ext(v, range);
    });
  });

  EXPECT_THAT(range.getRequiredBits() + 1, ::testing::Lt(8));
  EXPECT_THAT(ctx.getBufferSize(), Eq(1));
  EXPECT_THAT(t1.value(), Eq(r1.value()));
}
