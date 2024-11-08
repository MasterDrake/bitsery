// MIT License
//
// Copyright (c) 2020 Mindaugas Vinkelis
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

#include <bitsery/ext/eastl_bitset.h>
#include <bitsery/ext/value_range.h>

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

using EastlBitset = bitsery::ext::EastlBitset;
using ValueRange = bitsery::ext::ValueRange<int>;

using testing::Eq;

TEST(SerializeExtensionEastlBitset, BitsetSmallerThanULongLong)
{
  SerializationContext ctx;

  eastl::bitset<31> data;
  data[2] = true;
  data[8] = true;
  data[15] = true;
  data[25] = true;
  data[30] = true;
  eastl::bitset<31> res;

  ctx.createSerializer().ext(data, EastlBitset{});
  ctx.createDeserializer().ext(res, EastlBitset{});
  EXPECT_THAT(res, Eq(data));
}

TEST(SerializeExtensionEastlBitset, BitsetSmallerThanULongLong2)
{
  SerializationContext ctx;

  eastl::bitset<9> data;
  data.set();
  eastl::bitset<9> res;

  ctx.createSerializer().ext(data, EastlBitset{});
  ctx.createDeserializer().ext(res, EastlBitset{});
  EXPECT_THAT(res, Eq(data));
}

TEST(SerializeExtensionEastlBitset, BitsetLargerThanULongLong)
{
  SerializationContext ctx;

  eastl::bitset<200> data;
  data[1] = true;
  data[31] = true;
  data[63] = true;
  data[100] = true;
  data[191] = true;
  eastl::bitset<200> res;

  ctx.createSerializer().ext(data, EastlBitset{});
  ctx.createDeserializer().ext(res, EastlBitset{});
  EXPECT_THAT(res, Eq(data));
}

TEST(SerializeExtensionEastlBitset, BitsetSmallerThanULongLongBitPackingEnabled)
{
  SerializationContext ctx;

  eastl::bitset<12> data;
  int other_data = 1001;
  data[2] = true;
  data[9] = true;
  eastl::bitset<12> res{};
  int other_res{};

  ctx.createSerializer().enableBitPacking(
    [&data, &other_data](SerializationContext::TSerializerBPEnabled& sbp) {
      sbp.ext(data, EastlBitset{});
      sbp.ext(other_data, ValueRange{ 1000, 1015 });
    });
  ctx.createDeserializer().enableBitPacking(
    [&res, &other_res](SerializationContext::TDeserializerBPEnabled& dbp) {
      dbp.ext(res, EastlBitset{});
      dbp.ext(other_res, ValueRange{ 1000, 1015 });
    });
  EXPECT_THAT(res, Eq(data));
  EXPECT_THAT(other_res, Eq(other_data));
  EXPECT_THAT(ctx.getBufferSize(), Eq(2));
}

TEST(SerializeExtensionEastlBitset, BitsetLargerThanULongLongBitPackingEnabled)
{
  SerializationContext ctx;

  eastl::bitset<204> data;
  int other_data = 1001;
  data[1] = true;
  data[100] = true;
  data[191] = true;
  eastl::bitset<204> res{};
  int other_res{};

  ctx.createSerializer().enableBitPacking(
    [&data, &other_data](SerializationContext::TSerializerBPEnabled& sbp) {
      sbp.ext(data, EastlBitset{});
      sbp.ext(other_data, ValueRange{ 1000, 1015 });
    });
  ctx.createDeserializer().enableBitPacking(
    [&res, &other_res](SerializationContext::TDeserializerBPEnabled& dbp) {
      dbp.ext(res, EastlBitset{});
      dbp.ext(other_res, ValueRange{ 1000, 1015 });
    });
  EXPECT_THAT(res, Eq(data));
  EXPECT_THAT(other_res, Eq(other_data));
  EXPECT_THAT(ctx.getBufferSize(), Eq(26));
}
