// MIT License
//
// Copyright (c) 2018 Mindaugas Vinkelis
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

#ifndef BITSERY_EXT_EASTL_SMART_PTR_H
#define BITSERY_EXT_EASTL_SMART_PTR_H

#include "../traits/core/traits.h"
#include "utils/pointer_utils.h"
#include "utils/polymorphism_utils.h"
#include "utils/rtti_utils.h"

#include "EASTL/shared_ptr.h"
#include "EASTL/unique_ptr.h"

namespace bitsery {
namespace ext {

namespace smart_ptr_details {

// further code is for managing shared ownership
// do not nest this type in pointer manager class itself, because it will be
// different type for different T
struct SharedPtrSharedState : pointer_utils::PointerSharedStateBase
{
  eastl::shared_ptr<void> obj{};
};

template<typename T>
struct SmartPtrOwnerManager
{

  using TElement = typename T::element_type;

  template<typename TDeleter>
  static TElement* getPtr(eastl::unique_ptr<TElement, TDeleter>& obj)
  {
    return obj.get();
  }

  static TElement* getPtr(eastl::shared_ptr<TElement>& obj) { return obj.get(); }

  static TElement* getPtr(eastl::weak_ptr<TElement>& obj)
  {
    if (auto ptr = obj.lock())
      return ptr.get();
    return nullptr;
  }

  static constexpr PointerOwnershipType getOwnership()
  {
    return ::bitsery::details::IsSpecializationOf<T, eastl::unique_ptr>::value
             ? PointerOwnershipType::Owner
           : eastl::is_same<eastl::shared_ptr<TElement>, T>::value
             ? PointerOwnershipType::SharedOwner
             : PointerOwnershipType::SharedObserver;
  }

  template<typename TDeleter>
  static void create(eastl::unique_ptr<TElement, TDeleter>& obj,
                     pointer_utils::PolyAllocWithTypeId alloc,
                     size_t typeId)
  {
    obj.reset(alloc.newObject<TElement>(typeId));
  }

  template<typename TDeleter>
  static void createPolymorphic(
    eastl::unique_ptr<TElement, TDeleter>& obj,
    pointer_utils::PolyAllocWithTypeId alloc,
    const eastl::shared_ptr<PolymorphicHandlerBase>& handler)
  {
    obj.reset(static_cast<TElement*>(handler->create(alloc)));
  }

  template<typename TDel>
  static void destroy(eastl::unique_ptr<TElement, TDel>& obj,
                      pointer_utils::PolyAllocWithTypeId alloc,
                      size_t typeId)
  {
    auto ptr = obj.release();
    alloc.deleteObject(ptr, typeId);
  }

  template<typename TDel>
  static void destroyPolymorphic(
    eastl::unique_ptr<TElement, TDel>& obj,
    pointer_utils::PolyAllocWithTypeId alloc,
    const eastl::shared_ptr<PolymorphicHandlerBase>& handler)
  {
    auto ptr = obj.release();
    handler->destroy(alloc, ptr);
  }

  static void destroy(eastl::shared_ptr<TElement>& obj, MemResourceBase*, size_t)
  {
    obj.reset();
  }

  static void destroyPolymorphic(eastl::shared_ptr<TElement>& obj,
                                 MemResourceBase*,
                                 const eastl::shared_ptr<PolymorphicHandlerBase>&)
  {
    obj.reset();
  }

  static void destroy(eastl::weak_ptr<TElement>& obj, MemResourceBase*, size_t)
  {
    obj.reset();
  }

  static void destroyPolymorphic(eastl::weak_ptr<TElement>& obj,
                                 MemResourceBase*,
                                 const eastl::shared_ptr<PolymorphicHandlerBase>&)
  {
    obj.reset();
  }

  // define a type that will store shared state for shared and weak ptrs
  using TSharedState = SharedPtrSharedState;

  static void createShared(TSharedState& state,
                           eastl::shared_ptr<TElement>& obj,
                           MemResourceBase* memResource,
                           size_t typeId)
  {
    // capture deleter parameters by value
    pointer_utils::PolyAllocWithTypeId alloc{ memResource };
    obj.reset(
      alloc.newObject<TElement>(typeId),
      [alloc, typeId](TElement* data) { alloc.deleteObject(data, typeId); },
      pointer_utils::StdPolyAlloc<TElement>(memResource));
    state.obj = obj;
  }

  static void createSharedPolymorphic(
    TSharedState& state,
    eastl::shared_ptr<TElement>& obj,
    MemResourceBase* memResource,
    const eastl::shared_ptr<PolymorphicHandlerBase>& handler)
  {
    // capture deleter parameters by value
    pointer_utils::PolyAllocWithTypeId alloc{ memResource };
    obj.reset(
      static_cast<TElement*>(handler->create(alloc)),
      [alloc, handler](TElement* data) { handler->destroy(alloc, data); },
      pointer_utils::StdPolyAlloc<TElement>(memResource));
    state.obj = obj;
  }

  static void createShared(TSharedState& state,
                           eastl::weak_ptr<TElement>& obj,
                           MemResourceBase* memResource,
                           size_t typeId)
  {
    pointer_utils::PolyAllocWithTypeId alloc{ memResource };
    eastl::shared_ptr<TElement> res(
      alloc.newObject<TElement>(typeId),
      [alloc, typeId](TElement* data) { alloc.deleteObject(data, typeId); },
      pointer_utils::StdPolyAlloc<TElement>(memResource));
    obj = res;
    state.obj = res;
  }

  static void createSharedPolymorphic(
    TSharedState& state,
    eastl::weak_ptr<TElement>& obj,
    MemResourceBase* memResource,
    const eastl::shared_ptr<PolymorphicHandlerBase>& handler)
  {
    pointer_utils::PolyAllocWithTypeId alloc{ memResource };
    eastl::shared_ptr<TElement> res(
      static_cast<TElement*>(handler->create(alloc)),
      [alloc, handler](TElement* data) { handler->destroy(alloc, data); },
      pointer_utils::StdPolyAlloc<TElement>(memResource));
    obj = res;
    state.obj = res;
  }

  static void saveToSharedState(TSharedState& state, T& obj)
  {
    state.obj = eastl::shared_ptr<TElement>(obj);
  }

  static void loadFromSharedState(TSharedState& state, T& obj)
  {
    // reinterpret_pointer_cast is only since c++17
    auto p = reinterpret_cast<TElement*>(state.obj.get());
    obj = eastl::shared_ptr<TElement>(state.obj, p);
  }
};
}

template<typename RTTI>
using EastlSmartPtrBase = pointer_utils::PointerObjectExtensionBase<
  smart_ptr_details::SmartPtrOwnerManager,
  PolymorphicContext,
  RTTI>;

// helper type for convienience
using EastlSmartPtr = EastlSmartPtrBase<StandardRTTI>;

}

namespace traits {

template<typename T, typename RTTI>
struct ExtensionTraits<ext::EastlSmartPtrBase<RTTI>, T>
{
  using TValue = typename T::element_type;
  static constexpr bool SupportValueOverload = true;
  static constexpr bool SupportObjectOverload = true;
  // if underlying type is not polymorphic, then we can enable lambda syntax
  static constexpr bool SupportLambdaOverload =
    !RTTI::template isPolymorphic<TValue>();
};

}

}

#endif // BITSERY_EXT_EASTL_SMART_PTR_H
