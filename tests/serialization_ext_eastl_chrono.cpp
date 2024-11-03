// MIT License
//
// Copyright (c) 2019 Mindaugas Vinkelis
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

#include <bitsery/ext/eastl_chrono.h>

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

using EastlDuration = bitsery::ext::EastlDuration;
using EastlTimePoint = bitsery::ext::EastlTimePoint;

using testing::Eq;

TEST(SerializeExtensionEastlChrono, IntegralDuration)
{
  SerializationContext ctx1;
  using Hours = eastl::chrono::duration<int32_t, eastl::ratio<60>>;

  Hours data{ 43 };
  Hours res{};

  ctx1.createSerializer().ext4b(data, EastlDuration{});
  ctx1.createDeserializer().ext4b(res, EastlDuration{});
  EXPECT_THAT(res, Eq(data));
}

TEST(SerializeExtensionEastlChrono, IntegralTimePoint)
{
  SerializationContext ctx1;
  using Duration = eastl::chrono::duration<int64_t, eastl::milli>;
  using TP = eastl::chrono::time_point<eastl::chrono::system_clock, Duration>;

  TP data{ Duration{ 243 } };
  TP res{};

  ctx1.createSerializer().ext8b(data, EastlTimePoint{});
  ctx1.createDeserializer().ext8b(res, EastlTimePoint{});
  EXPECT_THAT(res, Eq(data));
}

TEST(SerializeExtensionEastlChrono, FloatDuration)
{
  SerializationContext ctx1;
  using Hours = eastl::chrono::duration<float, eastl::ratio<60>>;

  Hours data{ 43.5f };
  Hours res{};

  ctx1.createSerializer().ext4b(data, EastlDuration{});
  ctx1.createDeserializer().ext4b(res, EastlDuration{});
  EXPECT_THAT(res, Eq(data));
}

TEST(SerializeExtensionEastlChrono, FloatTimePoint)
{
  SerializationContext ctx1;
  using Duration = eastl::chrono::duration<double, eastl::milli>;
  using TP = eastl::chrono::time_point<eastl::chrono::system_clock, Duration>;

  TP data{ Duration{ 243457.4 } };
  TP res{};

  ctx1.createSerializer().ext8b(data, EastlTimePoint{});
  ctx1.createDeserializer().ext8b(res, EastlTimePoint{});
  EXPECT_THAT(res, Eq(data));
}
