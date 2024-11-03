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

#include <bitsery/ext/inheritance.h>
#include <bitsery/ext/pointer.h>
#include <bitsery/ext/eastl_smart_ptr.h>

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

using bitsery::ext::BaseClass;
using bitsery::ext::VirtualBaseClass;

using bitsery::ext::InheritanceContext;
using bitsery::ext::PointerLinkingContext;
using bitsery::ext::PointerType;
using bitsery::ext::PolymorphicContext;
using bitsery::ext::StandardRTTI;

using bitsery::ext::PointerObserver;
using bitsery::ext::EastlSmartPtr;

using testing::Eq;
using testing::Ne;

struct Base
{
  uint8_t x{};

  virtual ~Base() = default;
};

template<typename S>
void
serialize(S& s, Base& o)
{
  s.value1b(o.x);
}

struct Derived : virtual Base
{
  uint8_t y{};

  Derived() = default;

  Derived(uint8_t x_, uint8_t y_)
  {
    x = x_;
    y = y_;
  }
};

template<typename S>
void
serialize(S& s, Derived& o)
{
  s.ext(o, VirtualBaseClass<Base>{});
  s.value1b(o.y);
}

struct MoreDerived : Derived
{
  uint8_t z{};

  MoreDerived() = default;

  MoreDerived(uint8_t x_, uint8_t y_, uint8_t z_)
    : Derived(x_, y_)
  {
    z = z_;
  }
};

template<typename S>
void
serialize(S& s, MoreDerived& o)
{
  s.ext(o, BaseClass<Derived>{});
  s.value1b(o.z);
}

// define relationships between base class and derived classes for runtime
// polymorphism

namespace bitsery {
namespace ext {

template<>
struct PolymorphicBaseClass<Base> : PolymorphicDerivedClasses<Derived>
{
};

template<>
struct PolymorphicBaseClass<Derived> : PolymorphicDerivedClasses<MoreDerived>
{
};

}
}

template<typename T>
class SerializeExtensionEastlSmartPtrNonPolymorphicType : public testing::Test
{
public:
  template<typename U>
  using TPtr = typename T::template TData<U>;
  using TExt = typename T::TExt;

  using TContext = eastl::tuple<PointerLinkingContext>;
  using SerContext = BasicSerializationContext<TContext>;

  // this is useful for PolymorphicContext to bind classes to
  // serializer/deserializer
  using TSerializer = typename SerContext::TSerializer;
  using TDeserializer = typename SerContext::TDeserializer;

  TContext plctx{};
  SerContext sctx{};

  typename SerContext::TSerializer& createSerializer()
  {
    return sctx.createSerializer(plctx);
  }

  typename SerContext::TDeserializer& createDeserializer()
  {
    return sctx.createDeserializer(plctx);
  }

  bool isPointerContextValid() { return eastl::get<0>(plctx).isValid(); }

  virtual void TearDown() override { EXPECT_TRUE(isPointerContextValid()); }
};

template<typename T>
class SerializeExtensionEastlSmartPtrPolymorphicType : public testing::Test
{
public:
  template<typename U>
  using TPtr = typename T::template TData<U>;
  using TExt = typename T::TExt;

  using TContext = eastl::tuple<PointerLinkingContext,
                              InheritanceContext,
                              PolymorphicContext<StandardRTTI>>;
  using SerContext = BasicSerializationContext<TContext>;

  // this is useful for PolymorphicContext to bind classes to
  // serializer/deserializer
  using TSerializer = typename SerContext::TSerializer;
  using TDeserializer = typename SerContext::TDeserializer;

  TContext plctx{};
  SerContext sctx{};

  typename SerContext::TSerializer& createSerializer()
  {
    auto& res = sctx.createSerializer(plctx);
    eastl::get<2>(plctx).clear();
    // bind serializer with classes
    eastl::get<2>(plctx).template registerBasesList<SerContext::TSerializer>(
      bitsery::ext::PolymorphicClassesList<Base>{});
    return res;
  }

  typename SerContext::TDeserializer& createDeserializer()
  {
    auto& res = sctx.createDeserializer(plctx);
    eastl::get<2>(plctx).clear();
    // bind deserializer with classes
    eastl::get<2>(plctx).template registerBasesList<SerContext::TDeserializer>(
      bitsery::ext::PolymorphicClassesList<Base>{});
    return res;
  }

  bool isPointerContextValid() { return eastl::get<0>(plctx).isValid(); }

  virtual void TearDown() override { EXPECT_TRUE(isPointerContextValid()); }
};

struct UniquePtrTest
{
  template<typename T>
  using TData = eastl::unique_ptr<T>;
  using TExt = EastlSmartPtr;
};

struct SharedPtrTest
{
  template<typename T>
  using TData = eastl::shared_ptr<T>;
  using TExt = EastlSmartPtr;
};

using TestingWithNonPolymorphicTypes =
  ::testing::Types<UniquePtrTest, SharedPtrTest>;

TYPED_TEST_SUITE(SerializeExtensionEastlSmartPtrNonPolymorphicType,
                 TestingWithNonPolymorphicTypes, );

using TestingWithPolymorphicTypes =
  ::testing::Types<UniquePtrTest, SharedPtrTest>;

TYPED_TEST_SUITE(SerializeExtensionEastlSmartPtrPolymorphicType,
                 TestingWithPolymorphicTypes, );

TYPED_TEST(SerializeExtensionEastlSmartPtrNonPolymorphicType, Data0Result0)
{
  using Ptr = typename TestFixture::template TPtr<MyStruct1>;
  using Ext = typename TestFixture::TExt;

  Ptr data{};
  this->createSerializer().ext(data, Ext{});
  Ptr res{};
  this->createDeserializer().ext(res, Ext{});

  EXPECT_THAT(data.get(), ::testing::IsNull());
  EXPECT_THAT(res.get(), ::testing::IsNull());
}

TYPED_TEST(SerializeExtensionEastlSmartPtrNonPolymorphicType, Data0Result1)
{
  using Ptr = typename TestFixture::template TPtr<MyStruct1>;
  using Ext = typename TestFixture::TExt;

  Ptr data{};
  this->createSerializer().ext(data, Ext{});
  Ptr res{ new MyStruct1{} };
  this->createDeserializer().ext(res, Ext{});

  EXPECT_THAT(data.get(), ::testing::IsNull());
  EXPECT_THAT(res.get(), ::testing::IsNull());
}

TYPED_TEST(SerializeExtensionEastlSmartPtrNonPolymorphicType, Data1Result0)
{
  using Ptr = typename TestFixture::template TPtr<MyStruct1>;
  using Ext = typename TestFixture::TExt;

  Ptr data{ new MyStruct1{ 3, 78 } };
  this->createSerializer().ext(data, Ext{});
  Ptr res{};
  this->createDeserializer().ext(res, Ext{});

  EXPECT_THAT(data.get(), ::testing::NotNull());
  EXPECT_THAT(res.get(), ::testing::NotNull());
  EXPECT_THAT(res->i1, Eq(data->i1));
  EXPECT_THAT(res->i2, Eq(data->i2));
}

TYPED_TEST(SerializeExtensionEastlSmartPtrNonPolymorphicType, Data1Result1)
{
  using Ptr = typename TestFixture::template TPtr<MyStruct1>;
  using Ext = typename TestFixture::TExt;

  Ptr data{ new MyStruct1{ 3, 78 } };
  this->createSerializer().ext(data, Ext{});
  Ptr res{ new MyStruct1{} };
  this->createDeserializer().ext(res, Ext{});

  EXPECT_THAT(data.get(), ::testing::NotNull());
  EXPECT_THAT(res.get(), ::testing::NotNull());
  EXPECT_THAT(res->i1, Eq(data->i1));
  EXPECT_THAT(res->i2, Eq(data->i2));
}

TYPED_TEST(SerializeExtensionEastlSmartPtrNonPolymorphicType,
           CanUseLambdaOverload)
{
  using Ptr = typename TestFixture::template TPtr<MyStruct1>;
  using Ext = typename TestFixture::TExt;

  Ptr data{ new MyStruct1{ 3, 78 } };
  auto& ser = this->createSerializer();
  ser.ext(data, Ext{}, [](decltype(ser)& ser, MyStruct1& o) {
    // serialize only one field
    ser.value4b(o.i1);
  });
  Ptr res{ new MyStruct1{ 97, 12 } };
  auto& des = this->createDeserializer();
  des.ext(
    res, Ext{}, [](decltype(des)& des, MyStruct1& o) { des.value4b(o.i1); });

  EXPECT_THAT(res->i1, Eq(data->i1));
  EXPECT_THAT(res->i2, Ne(data->i2));
}

TYPED_TEST(SerializeExtensionEastlSmartPtrNonPolymorphicType, CanUseValueOverload)
{
  using Ptr = typename TestFixture::template TPtr<uint16_t>;
  using Ext = typename TestFixture::TExt;

  Ptr data{ new uint16_t{ 3 } };
  this->createSerializer().ext2b(data, Ext{});
  Ptr res{};
  this->createDeserializer().ext2b(res, Ext{});
  EXPECT_THAT(*res, Eq(*data));
}

TYPED_TEST(SerializeExtensionEastlSmartPtrNonPolymorphicType,
           FirstPtrThenPointerObserver)
{
  using Ptr = typename TestFixture::template TPtr<uint16_t>;
  using Ext = typename TestFixture::TExt;

  Ptr data{ new uint16_t{ 3 } };
  uint16_t* dataObs = data.get();
  auto& ser = this->createSerializer();
  ser.ext2b(data, Ext{});
  ser.ext2b(dataObs, PointerObserver{});
  Ptr res{};
  uint16_t* resObs = nullptr;
  auto& des = this->createDeserializer();
  des.ext2b(res, Ext{});
  des.ext2b(resObs, PointerObserver{});

  EXPECT_THAT(resObs, Eq(res.get()));
}

TYPED_TEST(SerializeExtensionEastlSmartPtrNonPolymorphicType,
           FirstPointerObserverThenPtr)
{
  using Ptr = typename TestFixture::template TPtr<uint16_t>;
  using Ext = typename TestFixture::TExt;

  Ptr data{ new uint16_t{ 3 } };
  uint16_t* dataObs = data.get();
  auto& ser = this->createSerializer();
  ser.ext2b(dataObs, PointerObserver{});
  ser.ext2b(data, Ext{});
  Ptr res{};
  uint16_t* resObs = nullptr;
  auto& des = this->createDeserializer();
  des.ext2b(resObs, PointerObserver{});
  des.ext2b(res, Ext{});
  EXPECT_THAT(resObs, Eq(res.get()));
}

TYPED_TEST(SerializeExtensionEastlSmartPtrPolymorphicType, Data0Result0)
{
  using Ptr = typename TestFixture::template TPtr<Base>;
  using Ext = typename TestFixture::TExt;

  Ptr baseData{};
  this->createSerializer().ext(baseData, Ext{});
  Ptr baseRes{};
  this->createDeserializer().ext(baseRes, Ext{});

  EXPECT_THAT(baseRes.get(), ::testing::IsNull());
  EXPECT_THAT(baseData.get(), ::testing::IsNull());
}

TYPED_TEST(SerializeExtensionEastlSmartPtrPolymorphicType, Data0Result1)
{
  using Ptr = typename TestFixture::template TPtr<Base>;
  using Ext = typename TestFixture::TExt;

  Ptr baseData{};
  this->createSerializer().ext(baseData, Ext{});

  Ptr baseRes{ new Derived{} };
  this->createDeserializer().ext(baseRes, Ext{});

  EXPECT_THAT(baseRes.get(), ::testing::IsNull());
  EXPECT_THAT(baseData.get(), ::testing::IsNull());
}

TYPED_TEST(SerializeExtensionEastlSmartPtrPolymorphicType, Data1Result0)
{
  using Ptr = typename TestFixture::template TPtr<Base>;
  using Ext = typename TestFixture::TExt;

  Ptr baseData{ new Derived{ 3, 78 } };
  this->createSerializer().ext(baseData, Ext{});
  Ptr baseRes{};
  this->createDeserializer().ext(baseRes, Ext{});

  auto* data = dynamic_cast<Derived*>(baseData.get());
  auto* res = dynamic_cast<Derived*>(baseRes.get());

  EXPECT_THAT(data, ::testing::NotNull());
  EXPECT_THAT(res, ::testing::NotNull());
  EXPECT_THAT(res->x, Eq(data->x));
  EXPECT_THAT(res->y, Eq(data->y));
}

TYPED_TEST(SerializeExtensionEastlSmartPtrPolymorphicType,
           DataAndResultWithDifferentRuntimeTypes)
{
  using Ptr = typename TestFixture::template TPtr<Base>;
  using Ext = typename TestFixture::TExt;

  Ptr baseData{ new Derived{ 3, 78 } };
  this->createSerializer().ext(baseData, Ext{});
  Ptr baseRes{ new Base{} };
  this->createDeserializer().ext(baseRes, Ext{});

  auto* data = dynamic_cast<Derived*>(baseData.get());
  auto* res = dynamic_cast<Derived*>(baseRes.get());

  EXPECT_THAT(data, ::testing::NotNull());
  EXPECT_THAT(res, ::testing::NotNull());
  EXPECT_THAT(res->x, Eq(data->x));
  EXPECT_THAT(res->y, Eq(data->y));
}

class SerializeExtensionEastlSmartSharedPtr : public testing::Test
{
public:
  using TContext = eastl::tuple<PointerLinkingContext,
                              InheritanceContext,
                              PolymorphicContext<StandardRTTI>>;
  using SerContext = BasicSerializationContext<TContext>;

  // this is useful for PolymorphicContext to bind classes to
  // serializer/deserializer
  using TSerializer = typename SerContext::TSerializer;
  using TDeserializer = typename SerContext::TDeserializer;

  TContext plctx{};
  SerContext sctx{};

  typename SerContext::TSerializer& createSerializer()
  {
    auto& res = sctx.createSerializer(plctx);
    eastl::get<2>(plctx).clear();
    // bind serializer with classes
    eastl::get<2>(plctx).registerBasesList<SerContext::TSerializer>(
      bitsery::ext::PolymorphicClassesList<Base>{});
    return res;
  }

  typename SerContext::TDeserializer& createDeserializer()
  {
    auto& res = sctx.createDeserializer(plctx);
    eastl::get<2>(plctx).clear();
    // bind deserializer with classes
    eastl::get<2>(plctx).registerBasesList<SerContext::TDeserializer>(
      bitsery::ext::PolymorphicClassesList<Base>{});
    return res;
  }

  size_t getBufferSize() const { return sctx.getBufferSize(); }

  bool isPointerContextValid() { return eastl::get<0>(plctx).isValid(); }

  void clearSharedState() { return eastl::get<0>(plctx).clearSharedState(); }
};

TEST_F(SerializeExtensionEastlSmartSharedPtr, SameSharedObjectIsSerializedOnce)
{

  eastl::shared_ptr<Base> baseData1{ new Derived{ 3, 78 } };
  eastl::shared_ptr<Base> baseData2{ baseData1 };
  auto& ser = createSerializer();
  ser.ext(baseData1, EastlSmartPtr{});
  ser.ext(baseData1, EastlSmartPtr{});
  createDeserializer();

  // 1b linking context (for 1st time)
  // 1b dynamic type info
  // 2b Derived object
  // 1b linking context (for 2nd time)
  EXPECT_THAT(getBufferSize(), Eq(5));
  EXPECT_TRUE(isPointerContextValid());
}

TEST_F(SerializeExtensionEastlSmartSharedPtr,
       PointerLinkingContextCorrectlyClearSharedState)
{

  eastl::shared_ptr<Base> baseData1{ new Derived{ 3, 78 } };

  auto& ser = createSerializer();
  ser.ext(baseData1, EastlSmartPtr{});
  eastl::shared_ptr<Base> baseRes1{};
  auto& des = createDeserializer();
  des.ext(baseRes1, EastlSmartPtr{});
  EXPECT_THAT(baseRes1.use_count(), Eq(2));
  clearSharedState();
  EXPECT_THAT(baseRes1.use_count(), Eq(1));
  EXPECT_TRUE(isPointerContextValid());
}

TEST_F(SerializeExtensionEastlSmartSharedPtr, CorrectlyManagesSameSharedObject)
{

  eastl::shared_ptr<Base> baseData1{ new Derived{ 3, 78 } };
  eastl::shared_ptr<Base> baseData2{ new Derived{ 55, 11 } };
  eastl::shared_ptr<Base> baseData21{ baseData2 };
  auto& ser = createSerializer();
  ser.ext(baseData1, EastlSmartPtr{});
  ser.ext(baseData2, EastlSmartPtr{});
  ser.ext(baseData21, EastlSmartPtr{});

  eastl::shared_ptr<Base> baseRes1{};
  eastl::shared_ptr<Base> baseRes2{};
  eastl::shared_ptr<Base> baseRes21{};
  auto& des = createDeserializer();
  des.ext(baseRes1, EastlSmartPtr{});
  des.ext(baseRes2, EastlSmartPtr{});
  des.ext(baseRes21, EastlSmartPtr{});

  auto* data = dynamic_cast<Derived*>(baseRes1.get());
  EXPECT_THAT(data, ::testing::NotNull());

  clearSharedState();

  EXPECT_THAT(baseRes1.use_count(), Eq(1));
  EXPECT_THAT(baseRes2.use_count(), Eq(2));
  EXPECT_THAT(baseRes21.use_count(), Eq(2));
  baseRes2.reset();
  EXPECT_THAT(baseRes21.use_count(), Eq(1));
  EXPECT_TRUE(isPointerContextValid());
}

TEST_F(SerializeExtensionEastlSmartSharedPtr, FirstSharedThenWeakPtr)
{

  eastl::shared_ptr<Base> baseData1{ new Derived{ 3, 78 } };
  eastl::weak_ptr<Base> baseData11{ baseData1 };
  eastl::weak_ptr<Base> baseData12{ baseData11 };
  auto& ser = createSerializer();
  ser.ext(baseData1, EastlSmartPtr{});
  ser.ext(baseData11, EastlSmartPtr{});
  ser.ext(baseData12, EastlSmartPtr{});

  eastl::shared_ptr<Base> baseRes1{};
  eastl::weak_ptr<Base> baseRes11{};
  eastl::weak_ptr<Base> baseRes12{};
  auto& des = createDeserializer();
  des.ext(baseRes1, EastlSmartPtr{});
  des.ext(baseRes11, EastlSmartPtr{});
  des.ext(baseRes12, EastlSmartPtr{});

  auto* data = dynamic_cast<Derived*>(baseRes1.get());
  EXPECT_THAT(data, ::testing::NotNull());

  clearSharedState();

  EXPECT_THAT(baseRes1.use_count(), Eq(1));
  EXPECT_THAT(baseRes11.use_count(), Eq(1));
  EXPECT_THAT(baseRes12.use_count(), Eq(1));
  baseRes1.reset();
  EXPECT_THAT(baseRes11.use_count(), Eq(0));
  EXPECT_TRUE(isPointerContextValid());
}

TEST_F(SerializeExtensionEastlSmartSharedPtr, FirstWeakThenSharedPtr)
{

  eastl::shared_ptr<MyStruct1> baseData1{ new MyStruct1{ 3, 78 } };
  eastl::weak_ptr<MyStruct1> baseData11{ baseData1 };
  eastl::weak_ptr<MyStruct1> baseData2{};
  auto& ser = createSerializer();
  ser.ext(baseData2, EastlSmartPtr{});
  ser.ext(baseData11, EastlSmartPtr{});
  ser.ext(baseData1, EastlSmartPtr{});

  eastl::shared_ptr<MyStruct1> baseRes1{};
  eastl::weak_ptr<MyStruct1> baseRes11{};
  eastl::weak_ptr<MyStruct1> baseRes2{};
  auto& des = createDeserializer();
  des.ext(baseRes2, EastlSmartPtr{});
  des.ext(baseRes11, EastlSmartPtr{});
  des.ext(baseRes1, EastlSmartPtr{});

  clearSharedState();

  EXPECT_THAT(*baseData1, Eq(*baseRes1));
  EXPECT_THAT(baseRes1.use_count(), Eq(1));
  EXPECT_THAT(baseRes2.use_count(), Eq(0));
  EXPECT_THAT(baseRes11.use_count(), Eq(1));
  baseRes1.reset();
  EXPECT_THAT(baseRes11.use_count(), Eq(0));
  EXPECT_TRUE(isPointerContextValid());
}

TEST_F(SerializeExtensionEastlSmartSharedPtr, WeakPtrFirstPolymorphicData0Result1)
{

  eastl::shared_ptr<Base> baseData1{};
  eastl::weak_ptr<Base> baseData2{};
  auto& ser = createSerializer();
  ser.ext(baseData2, EastlSmartPtr{});
  ser.ext(baseData1, EastlSmartPtr{});

  eastl::shared_ptr<Base> baseRes1{ new Base{} };
  eastl::weak_ptr<Base> baseRes2{ baseRes1 };
  auto& des = createDeserializer();
  des.ext(baseRes2, EastlSmartPtr{});
  des.ext(baseRes1, EastlSmartPtr{});

  clearSharedState();

  EXPECT_THAT(baseRes1.use_count(), Eq(0));
  EXPECT_THAT(baseRes2.use_count(), Eq(0));
  baseRes1.reset();
  EXPECT_TRUE(isPointerContextValid());
}

TEST_F(SerializeExtensionEastlSmartSharedPtr,
       WeakPtrFirstNonPolymorphicData0Result1)
{

  eastl::shared_ptr<MyStruct2> baseData1{};
  eastl::weak_ptr<MyStruct2> baseData2{};
  auto& ser = createSerializer();
  ser.ext(baseData2, EastlSmartPtr{});
  ser.ext(baseData1, EastlSmartPtr{});

  eastl::shared_ptr<MyStruct2> baseRes1{ new MyStruct2{ MyStruct2::MyEnum::V4,
                                                      { 1, 87 } } };
  eastl::weak_ptr<MyStruct2> baseRes2{ baseRes1 };
  auto& des = createDeserializer();
  des.ext(baseRes2, EastlSmartPtr{});
  des.ext(baseRes1, EastlSmartPtr{});

  clearSharedState();

  EXPECT_THAT(baseRes1.use_count(), Eq(0));
  EXPECT_THAT(baseRes2.use_count(), Eq(0));
  baseRes1.reset();
  EXPECT_TRUE(isPointerContextValid());
}

TEST_F(SerializeExtensionEastlSmartSharedPtr, FewPtrsAreEmpty)
{

  eastl::shared_ptr<Base> baseData1{ new Derived{ 3, 78 } };
  eastl::shared_ptr<Base> baseData2{};
  eastl::weak_ptr<Base> baseData3{};
  eastl::weak_ptr<Base> baseData11{ baseData1 };
  auto& ser = createSerializer();
  ser.ext(baseData1, EastlSmartPtr{});
  ser.ext(baseData2, EastlSmartPtr{});
  ser.ext(baseData3, EastlSmartPtr{});
  ser.ext(baseData11, EastlSmartPtr{});

  eastl::shared_ptr<Base> baseRes1{};
  eastl::shared_ptr<Base> baseRes2{ new Derived{ 3, 78 } };
  eastl::weak_ptr<Base> baseRes3{ baseRes2 };
  eastl::weak_ptr<Base> baseRes11{};
  auto& des = createDeserializer();
  des.ext(baseRes1, EastlSmartPtr{});
  des.ext(baseRes2, EastlSmartPtr{});
  des.ext(baseRes3, EastlSmartPtr{});
  des.ext(baseRes11, EastlSmartPtr{});

  clearSharedState();

  EXPECT_THAT(baseRes1.use_count(), Eq(1));
  EXPECT_THAT(baseRes2.use_count(), Eq(0));
  EXPECT_THAT(baseRes3.use_count(), Eq(0));
  EXPECT_THAT(baseRes11.use_count(), Eq(1));
  baseRes1.reset();
  EXPECT_THAT(baseRes11.use_count(), Eq(0));
  EXPECT_TRUE(isPointerContextValid());
}

TEST_F(SerializeExtensionEastlSmartSharedPtr, WhenResultObjectExistsSameType)
{

  eastl::shared_ptr<Base> baseData1{ new Derived{ 3, 78 } };
  auto& ser = createSerializer();
  ser.ext(baseData1, EastlSmartPtr{});

  eastl::shared_ptr<Base> baseRes1{ new Derived{ 0, 0 } };
  auto& des = createDeserializer();
  des.ext(baseRes1, EastlSmartPtr{});

  clearSharedState();

  EXPECT_THAT(baseRes1.use_count(), Eq(1));
  EXPECT_THAT(baseRes1->x, Eq(baseData1->x));
  EXPECT_TRUE(isPointerContextValid());
}

TEST_F(SerializeExtensionEastlSmartSharedPtr, WhenResultObjectExistsDifferentType)
{

  eastl::shared_ptr<Base> baseData1{ new Derived{ 3, 78 } };
  auto& ser = createSerializer();
  ser.ext(baseData1, EastlSmartPtr{});

  eastl::shared_ptr<Base> baseRes1{ new Base{} };
  auto& des = createDeserializer();
  des.ext(baseRes1, EastlSmartPtr{});

  clearSharedState();

  EXPECT_THAT(baseRes1.use_count(), Eq(1));
  EXPECT_THAT(baseRes1->x, Eq(baseData1->x));
  EXPECT_THAT(dynamic_cast<Derived*>(baseRes1.get()), ::testing::NotNull());
  EXPECT_TRUE(isPointerContextValid());
}

TEST_F(SerializeExtensionEastlSmartSharedPtr,
       WhenOnlyWeakPtrIsSerializedThenPointerCointextIsInvalid)
{
  eastl::shared_ptr<Base> tmp{ new Derived{ 3, 78 } };
  eastl::weak_ptr<Base> baseData1{ tmp };
  auto& ser = createSerializer();
  ser.ext(baseData1, EastlSmartPtr{});

  EXPECT_FALSE(isPointerContextValid());
}

TEST_F(SerializeExtensionEastlSmartSharedPtr,
       WhenOnlyWeakPtrIsDeserializedThenPointerCointextIsInvalid)
{
  eastl::shared_ptr<Base> baseData1{ new Derived{ 3, 78 } };
  auto& ser = createSerializer();
  ser.ext(baseData1, EastlSmartPtr{});

  eastl::weak_ptr<Base> baseRes1{};
  auto& des = createDeserializer();
  des.ext(baseRes1, EastlSmartPtr{});

  EXPECT_FALSE(isPointerContextValid());

  EXPECT_THAT(baseRes1.use_count(), Eq(1));
  clearSharedState();
  EXPECT_THAT(baseRes1.use_count(), Eq(0));
}

struct TestSharedFromThis
  : public eastl::enable_shared_from_this<TestSharedFromThis>
{
  float x{};

  explicit TestSharedFromThis()
    : eastl::enable_shared_from_this<TestSharedFromThis>()
  {
  }

  template<typename S>
  void serialize(S& s)
  {
    s.value4b(x);
  }
};

TEST_F(SerializeExtensionEastlSmartSharedPtr, EnableSharedFromThis)
{
  eastl::shared_ptr<TestSharedFromThis> dataPtr(new TestSharedFromThis{});
  eastl::shared_ptr<TestSharedFromThis> resPtr{};
  createSerializer().ext(dataPtr, EastlSmartPtr{});
  createDeserializer().ext(resPtr, EastlSmartPtr{});
  clearSharedState();
  auto resPtr2 = resPtr->shared_from_this();
  EXPECT_THAT(resPtr->x, Eq(dataPtr->x));
  EXPECT_THAT(resPtr2.use_count(), Eq(2));
}

struct CustomDeleter
{
  void operator()(Base* p) { delete p; }
};

class SerializeExtensionEastlSmartUniquePtr
  : public SerializeExtensionEastlSmartSharedPtr
{};

TEST_F(SerializeExtensionEastlSmartUniquePtr, WithCustomDeleter)
{
  eastl::unique_ptr<Base, CustomDeleter> dataPtr(new Derived{ 87, 7 });
  eastl::unique_ptr<Base, CustomDeleter> resPtr{};
  createSerializer().ext(dataPtr, EastlSmartPtr{});
  createDeserializer().ext(resPtr, EastlSmartPtr{});
  clearSharedState();
  EXPECT_THAT(resPtr->x, Eq(dataPtr->x));
  EXPECT_THAT(dynamic_cast<Derived*>(resPtr.get()), ::testing::NotNull());
}
