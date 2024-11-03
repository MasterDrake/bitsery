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

#if __cplusplus > 201402L

using testing::Eq;

#include <bitsery/ext/eastl_variant.h>

template<typename T, size_t N>
using OverloadValue = bitsery::ext::OverloadValue<T, N>;

TEST(SerializeExtensionEastlVariant, UseSerializeFunction)
{

  eastl::variant<MyStruct1, MyStruct2> t1{ MyStruct1{ 978, 15 } };
  eastl::variant<MyStruct1, MyStruct2> r1{ MyStruct2{} };
  SerializationContext ctx;
  ctx.createSerializer().ext(t1, bitsery::ext::EastlVariant{});
  ctx.createDeserializer().ext(r1, bitsery::ext::EastlVariant{});
  EXPECT_THAT(t1, Eq(r1));
}

TEST(SerializeExtensionEastlVariant,
     WhenTwoIndicesWithSameTypeThenDeserializeCorrectIndex)
{

  eastl::variant<MyStruct1, MyStruct2, MyStruct1> t1{ eastl::in_place_index_t<2>{},
                                                    MyStruct1{ 978, 15 } };
  eastl::variant<MyStruct1, MyStruct2, MyStruct1> r1{ MyStruct2{} };
  SerializationContext ctx;
  ctx.createSerializer().ext(t1, bitsery::ext::EastlVariant{});
  ctx.createDeserializer().ext(r1, bitsery::ext::EastlVariant{});
  EXPECT_THAT(t1, Eq(r1));
}

TEST(SerializeExtensionEastlVariant, ValueTypesCanBeSerializedWithLambda)
{

  eastl::variant<float, char, MyStruct1> t1{ 5.6f };
  eastl::variant<float, char, MyStruct1> r1{ MyStruct1{} };
  SerializationContext ctx;
  auto fncFloat = [](auto& s, float& v) { s.value4b(v); };
  auto fncChar = [](auto& s, char& v) { s.value1b(v); };
  ctx.createSerializer().ext(t1, bitsery::ext::EastlVariant{ fncFloat, fncChar });
  ctx.createDeserializer().ext(r1,
                               bitsery::ext::EastlVariant{ fncFloat, fncChar });
  EXPECT_THAT(t1, Eq(r1));
}

TEST(SerializeExtensionEastlVariant,
     ValueTypesCanBeSerializedWithLambdaAndOrCallableObject)
{
  eastl::variant<float, char, MyStruct1> t1{ 'Z' };
  eastl::variant<float, char, MyStruct1> r1{ MyStruct1{} };
  SerializationContext ctx;
  auto fncFloat = [](auto& s, float& v) { s.value4b(v); };

  ctx.createSerializer().ext(
    t1, bitsery::ext::EastlVariant{ fncFloat, OverloadValue<char, 1>{} });
  ctx.createDeserializer().ext(
    r1, bitsery::ext::EastlVariant{ fncFloat, OverloadValue<char, 1>{} });
  EXPECT_THAT(t1, Eq(r1));
}

TEST(SerializeExtensionEastlVariant, CanOverloadDefaultSerializationFunction)
{
  eastl::variant<MyStruct2, MyStruct1, int32_t> t1{ MyStruct1{ 5, 9 } };
  eastl::variant<MyStruct2, MyStruct1, int32_t> r1{ MyStruct1{} };
  SerializationContext ctx;
  auto exec = [](auto& s, eastl::variant<MyStruct2, MyStruct1, int32_t>& o) {
    using S = decltype(s);
    s.ext(o,
          bitsery::ext::EastlVariant{ [](S& s, MyStruct1& v) {
                                     s.value4b(v.i1);
                                     // do not serialize other element, it
                                     // should be 0 (default)
                                   },
                                    OverloadValue<int32_t, 4>{} });
  };

  ctx.createSerializer().object(t1, exec);
  ctx.createDeserializer().object(r1, exec);
  EXPECT_THAT(eastl::get<1>(r1).i2, Eq(0));
}

struct NonDefaultConstructable
{
  explicit NonDefaultConstructable(float x)
    : _x{ x }
  {
  }

  float _x;

  bool operator==(const NonDefaultConstructable& rhs) const
  {
    return _x == rhs._x;
  }

private:
  friend class bitsery::Access;

  NonDefaultConstructable()
    : _x{ 0.0f } {};
};

TEST(SerializeExtensionEastlVariant, CanUseNonDefaultConstructableTypes)
{
  eastl::variant<NonDefaultConstructable, MyStruct1, int32_t> t1{
    NonDefaultConstructable{ 123.456f }
  };
  eastl::variant<NonDefaultConstructable, MyStruct1, int32_t> r1{ MyStruct1{} };
  SerializationContext ctx;

  auto exec = [](auto& s,
                 eastl::variant<NonDefaultConstructable, MyStruct1, int32_t>& o) {
    using S = decltype(s);
    s.ext(o,
          bitsery::ext::EastlVariant{
            [](S& s, NonDefaultConstructable& v) { s.value4b(v._x); },
            OverloadValue<int32_t, 4>{} });
  };

  ctx.createSerializer().object(t1, exec);
  ctx.createDeserializer().object(r1, exec);

  EXPECT_THAT(t1, Eq(r1));
}

TEST(SerializeExtensionEastlVariant, CorrectlyHandleMonoState)
{
  eastl::variant<eastl::monostate, NonDefaultConstructable, MyStruct1> t1{};
  eastl::variant<eastl::monostate, NonDefaultConstructable, MyStruct1> r1{};
  SerializationContext ctx;

  auto exec = [](auto& s, auto& o) {
    using S = decltype(s);
    s.ext(o,
          bitsery::ext::EastlVariant{
            [](S& s, NonDefaultConstructable& v) { s.value4b(v._x); },
          });
  };

  ctx.createSerializer().object(t1, exec);
  ctx.createDeserializer().object(r1, exec);

  EXPECT_THAT(t1, Eq(r1));
  eastl::variant<eastl::monostate> t2{};
  eastl::variant<eastl::monostate> r2{};
  SerializationContext ctx1;
  ctx1.createSerializer().ext(t2, bitsery::ext::EastlVariant{});
  ctx1.createDeserializer().ext(r2, bitsery::ext::EastlVariant{});
  EXPECT_THAT(t2, Eq(r2));
}

#elif defined(_MSC_VER)
#pragma message(                                                               \
  "C++17 and /Zc:__cplusplus option is required to enable eastl::variant tests")
#else
#pragma message("C++17 is required to enable eastl::variant tests")
#endif
