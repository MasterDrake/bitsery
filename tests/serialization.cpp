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

using testing::Eq;

TEST(Serialization, AdapterCanBeMovedInAndOut)
{
  Buffer buf{};
  bitsery::Serializer<Writer> ser1{ buf };
  ser1.object(MyStruct1{ 1, 2 });
  auto writeAdapter = eastl::move(ser1).adapter();
  bitsery::Serializer<Writer> ser2(eastl::move(writeAdapter));
  ser2.object(MyStruct1{ 3, 4 });
  auto writtenBytesCount = ser2.adapter().writtenBytesCount();
  EXPECT_THAT(writtenBytesCount, Eq(MyStruct1::SIZE + MyStruct1::SIZE));

  MyStruct1 res{};
  bitsery::Deserializer<Reader> des1{ buf.begin(), writtenBytesCount };
  des1.object(res);
  EXPECT_THAT(res, Eq(MyStruct1{ 1, 2 }));
  auto readerAdapter = eastl::move(des1).adapter();
  bitsery::Deserializer<Reader> des2(eastl::move(readerAdapter));
  des2.object(res);
  EXPECT_THAT(res, Eq(MyStruct1{ 3, 4 }));
  EXPECT_TRUE(des2.adapter().isCompletedSuccessfully());
}
