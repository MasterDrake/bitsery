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

#ifndef BITSERY_EXT_EASTL_VARIANT_H
#define BITSERY_EXT_EASTL_VARIANT_H

#include "../traits/core/traits.h"
#include "utils/composite_type_overloads.h"
#include <ESATL/variant.h>

namespace bitsery {
namespace ext {

template<typename... Overloads>
class EastlVariant
  : public details::CompositeTypeOverloadsUtils<eastl::variant, Overloads...>
{
public:
  template<typename Ser, typename Fnc, typename... Ts>
  void serialize(Ser& ser, const eastl::variant<Ts...>& obj, Fnc&&) const
  {
    auto index = obj.index();
    assert(index != eastl::variant_npos);
    details::writeSize(ser.adapter(), index);
    this->execIndex(index,
                    const_cast<eastl::variant<Ts...>&>(obj),
                    [this, &ser](auto& data, auto index) {
                      constexpr size_t Index = decltype(index)::value;
                      this->serializeType(ser, eastl::get<Index>(data));
                    });
  }

  template<typename Des, typename Fnc, typename... Ts>
  void deserialize(Des& des, eastl::variant<Ts...>& obj, Fnc&&) const
  {
    size_t index{};
    details::readSize(
      des.adapter(),
      index,
      sizeof...(Ts),
      eastl::integral_constant<bool, Des::TConfig::CheckDataErrors>{});
    this->execIndex(index, obj, [this, &des](auto& data, auto index) {
      constexpr size_t Index = decltype(index)::value;
      using TElem =
        typename eastl::variant_alternative<Index, eastl::variant<Ts...>>::type;

      // Reinitializing nontrivial types may be expensive especially when they
      // reference heap data, so if `data` is already holding the requested
      // variant then we'll deserialize into the existing object
      if constexpr (!eastl::is_trivial_v<TElem>) {
        if (auto item = eastl::get_if<Index>(&data)) {
          this->serializeType(des, *item);
          return;
        }
      }

      TElem item = ::bitsery::Access::create<TElem>();
      this->serializeType(des, item);
      data =
        eastl::variant<Ts...>(eastl::in_place_index_t<Index>{}, eastl::move(item));
    });
  }
};

// deduction guide
template<typename... Overloads>
EastlVariant(Overloads...) -> EastlVariant<Overloads...>;
}

// defines empty fuction, that handles monostate
template<typename S>
void
serialize(S&, eastl::monostate&)
{
}

namespace traits {

template<typename Variant, typename... Overloads>
struct ExtensionTraits<ext::EastlVariant<Overloads...>, Variant>
{
  static_assert(
    bitsery::details::IsSpecializationOf<Variant, eastl::variant>::value,
    "EastlVariant only works with eastl::variant");
  using TValue = void;
  static constexpr bool SupportValueOverload = false;
  static constexpr bool SupportObjectOverload = true;
  static constexpr bool SupportLambdaOverload = false;
};

}

}

#endif // BITSERY_EXT_EASTL_VARIANT_H
