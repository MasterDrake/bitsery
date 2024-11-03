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

#include <bitsery/ext/entropy.h>
#include <bitsery/ext/eastl_map.h>
#include <bitsery/traits/string.h>
#include <EASTL/map.h>

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

using EastlMap = bitsery::ext::EastlMap;

using testing::Eq;

template<typename Container>
Container
createData()
{
  return {};
}

template<>
eastl::unordered_map<eastl::string, MyStruct1>
createData<eastl::unordered_map<eastl::string, MyStruct1>>()
{
  return { eastl::make_pair("some key", MyStruct1{ 874, 456 }),
           eastl::make_pair("other key", MyStruct1{ -34, 8645 }),
           eastl::make_pair("secret key", MyStruct1{ -4878, 3468975 }) };
}

template<>
eastl::unordered_multimap<int32_t, float>
createData<eastl::unordered_multimap<int32_t, float>>()
{
  return { eastl::pair<int32_t, float>(545, 45.485f),
           eastl::pair<int32_t, float>(6748, -7891.5f),
           eastl::pair<int32_t, float>(845, -457.0f) };
}

template<>
eastl::map<MyEnumClass, MyStruct1>
createData<eastl::map<MyEnumClass, MyStruct1>>()
{
  return { eastl::make_pair(MyEnumClass::E3, MyStruct1{ 874, 456 }),
           eastl::make_pair(MyEnumClass::E6, MyStruct1{ -34, 8645 }),
           eastl::make_pair(MyEnumClass::E2, MyStruct1{ -4878, 3468975 }) };
}

template<>
eastl::multimap<int32_t, int64_t>
createData<eastl::multimap<int32_t, int64_t>>()
{
  return { // these are optimized with range and entropy
           eastl::pair<int32_t, int64_t>(-45, -984196845ll),
           eastl::pair<int32_t, int64_t>(54, 1ll),
           eastl::pair<int32_t, int64_t>(98, 3ll)
  };
}

template<typename T>
class SerializeExtensionEastlMap : public testing::Test
{
public:
  using TContainer = T;

  const TContainer src = createData<TContainer>();
  TContainer res{};
};

using SerializeExtensionEastlMapTypes =
  ::testing::Types<eastl::unordered_map<eastl::string, MyStruct1>,
                   eastl::unordered_multimap<int32_t, float>,
                   eastl::map<MyEnumClass, MyStruct1>,
                   eastl::multimap<int32_t, int64_t>>;

TYPED_TEST_SUITE(SerializeExtensionEastlMap, SerializeExtensionEastlMapTypes, );

namespace bitsery {

template<typename S>
void
serialize(S& s, eastl::unordered_map<eastl::string, MyStruct1>& o)
{
  s.ext(o, EastlMap{ 10 }, [](S& s, eastl::string& key, MyStruct1& value) {
    s.text1b(key, 100);
    s.object(value);
  });
}

template<typename S>
void
serialize(S& s, eastl::unordered_multimap<int32_t, float>& o)
{
  s.ext(o, EastlMap{ 10 }, [](S& s, int32_t& key, float& value) {
    s.value4b(key);
    s.value4b(value);
  });
}

template<typename S>
void
serialize(S& s, eastl::map<MyEnumClass, MyStruct1>& o)
{
  s.ext(o, EastlMap{ 10 }, [](S& s, MyEnumClass& key, MyStruct1& value) {
    s.value4b(key);
    s.object(value);
  });
}

template<typename S>
void
serialize(S& s, eastl::multimap<int32_t, int64_t>& o)
{
  s.ext(o, EastlMap{ 10 }, [](S& s, int32_t& key, int64_t& value) {
    s.enableBitPacking([&key, &value](typename S::BPEnabledType& sbp) {
      int64_t values[3]{ 1ll, 2ll, 3ll };
      sbp.ext(key, bitsery::ext::ValueRange<int32_t>{ -100, 100 });
      sbp.ext8b(value, bitsery::ext::Entropy<int64_t[3]>{ values });
    });
  });
}

}

TYPED_TEST(SerializeExtensionEastlMap, SerializeAndDeserializeEquals)
{
  SerializationContext ctx1;
  ctx1.createSerializer().object(this->src);
  ctx1.createDeserializer().object(this->res);
  EXPECT_THAT(this->res, Eq(this->src));
}
