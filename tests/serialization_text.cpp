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
#include <bitsery/traits/string.h>
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

using namespace testing;

TEST(SerializeText, BasicString)
{
  SerializationContext ctx;
  eastl::string t1 = "some random text";
  eastl::string res;

  ctx.createSerializer().text<sizeof(eastl::string::value_type)>(t1, 1000);
  ctx.createDeserializer().text<sizeof(eastl::string::value_type)>(res, 1000);

  EXPECT_THAT(res.c_str(), StrEq(t1.c_str()));
  EXPECT_THAT(res, ContainerEq(t1));
}

TEST(SerializeText, WhenSizeOfTypeNotEqualsOneThenSetSizeExplicitly)
{
  SerializationContext ctx;
  constexpr auto VSIZE = sizeof(char32_t);
  eastl::basic_string<char32_t> t1 = U"some random text";
  eastl::basic_string<char32_t> res;
  static_assert(VSIZE > 1,
                "on this system, all character types has sizeof == 1, cannot "
                "run this tests");

  ctx.createSerializer().text<VSIZE>(t1, 1000);
  ctx.createDeserializer().text<VSIZE>(res, 1000);

  EXPECT_THAT(res, ContainerEq(t1));
}

TEST(SerializeText, BasicStringUseSizeMethodNotNullterminatedLength)
{
  SerializationContext ctx;
  eastl::wstring t1(L"some random text\0xxxxxx", 20);
  eastl::wstring wres;
  constexpr auto VSIZE = sizeof(eastl::wstring::value_type);

  ctx.createSerializer().text<VSIZE>(t1, 1000);
  ctx.createDeserializer().text<VSIZE>(wres, 1000);

  EXPECT_THAT(wres.c_str(), StrEq(t1.c_str()));
  EXPECT_THAT(wres.size(), Eq(t1.size()));
  EXPECT_THAT(
    wres.size(),
    Gt(eastl::CharStrlen(t1.data())));

  SerializationContext ctx2;
  eastl::string t2("\0no one cares what is there", 10);
  eastl::string res;
  ctx2.createSerializer().text<sizeof(eastl::string::value_type)>(t2, 1000);
  ctx2.createDeserializer().text<sizeof(eastl::string::value_type)>(res, 1000);

  EXPECT_THAT(res.c_str(), StrEq(t2.c_str()));
  EXPECT_THAT(res.size(), Eq(t2.size()));

  SerializationContext ctx3;
  eastl::string t3("never ending buffer that doesnt fit in this string", 10);
  ctx3.createSerializer().text<sizeof(eastl::string::value_type)>(t3, 1000);
  ctx3.createDeserializer().text<sizeof(eastl::string::value_type)>(res, 1000);
  EXPECT_THAT(res.c_str(), StrEq(t3.c_str()));
  EXPECT_THAT(res.size(), Eq(10));
}

constexpr int CARR_LENGTH = 10;

TEST(SerializeText, CArraySerializesTextLength)
{
  SerializationContext ctx;
  char t1[CARR_LENGTH]{ "some text" };
  char r1[CARR_LENGTH]{};

  ctx.createSerializer().text<1>(t1);
  ctx.createDeserializer().text<1>(r1);

  EXPECT_THAT(ctx.getBufferSize(),
              Eq(ctx.containerSizeSerializedBytesCount(CARR_LENGTH) +
                 eastl::CharStrlen(t1)));

  EXPECT_THAT(r1, StrEq(t1));
  EXPECT_THAT(r1, ContainerEq(t1));

  // zero length string
  t1[0] = 0;
  SerializationContext ctx2;
  ctx2.createSerializer().text<1>(t1);
  ctx2.createDeserializer().text<1>(r1);

  EXPECT_THAT(ctx2.getBufferSize(),
              Eq(ctx2.containerSizeSerializedBytesCount(CARR_LENGTH)));
  EXPECT_THAT(r1, StrEq(t1));
  EXPECT_THAT(r1, ContainerEq(t1));
}

#ifndef NDEBUG
TEST(SerializeText, WhenCArrayNotNullterminatedThenAssert)
{
  SerializationContext ctx;
  char16_t t1[CARR_LENGTH]{ u"some text" };
  // make last character not nullterminated
  t1[CARR_LENGTH - 1] = 'x';
  EXPECT_DEATH(ctx.createSerializer().text<2>(t1), "");
}
#endif

TEST(SerializeText, WhenContainerOrTextSizeIsMoreThanMaxThenInvalidDataError)
{
  SerializationContext ctx;
  eastl::string tmp = "larger text then allowed";
  ctx.createSerializer().text1b(tmp, 100);
  ctx.createDeserializer().text1b(tmp, 10);
  EXPECT_THAT(ctx.des->adapter().error(),
              Eq(bitsery::ReaderError::InvalidData));
}