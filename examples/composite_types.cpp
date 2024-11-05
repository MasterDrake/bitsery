#include <bitsery/adapter/buffer.h>
#include <bitsery/bitsery.h>
#include <bitsery/traits/vector.h>
// include extensions to work with tuples and variants these extesions only work with C++17

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

#if __cplusplus > 201402L
#include <bitsery/ext/eastl_tuple.h>
#include <bitsery/ext/eastl_variant.h>
// let's include this extension to make it more interesting :)
#include <bitsery/ext/compact_value.h>

struct MyStruct
{
  eastl::vector<int32_t> v{};
  float f{};

  bool operator==(const MyStruct& rhs) const
  {
    return v == rhs.v && f == rhs.f;
  }
};

template<typename S>
void
serialize(S& s, MyStruct& o)
{
  s.container4b(o.v, 1000);
  s.value4b(o.f);
}

// this will be the type that we want to serialize/deserialize
using MyTuple = eastl::tuple<float, MyStruct>;
using MyVariant = eastl::variant<int64_t, MyTuple, MyStruct>;

// define default serialize function for MyVariant, so that we could use
// quickSerialization/Deserialization functions
template<typename S>
void
serialize(S& s, MyVariant& o)
{
  // in order to serialize a variant, it needs to know how to do it for all
  // types we can do this simply by providing any callable object, that accepts
  // serializer and type as arguments
  s.ext(
    o,
    bitsery::ext::EastlVariant{
      // specify how to serialize tuple by creating a lambda
      [](S& s, MyTuple& o) {
        // EastlTuple is used exactly the same as EastlVariant
        s.ext(
          o,
          bitsery::ext::EastlTuple{
            // this is convenient callable object to specify integral value size
            // it is different equivalent to lambda [](auto& s, float&o) {
            // s.value4b(o);}
            bitsery::ext::OverloadValue<float, 4>{},
            // it is not required to provide MyStruct overload, because it we
            // have defined 'serialize' function for it
          });
      },
      // this might also be useful if you want to overload using extension
      bitsery::ext::OverloadExtValue<int64_t, 8, bitsery::ext::CompactValue>{},
      // you can even go further and instead of writing lambda for MyTuple you
      // can as well compose the same functionality
      // with OverloadExtObject, like this:
      // (comment out MyTuple lambda, and uncomment this)
      // ext::OverloadExtObject<MyTuple, ext::EastlTuple<ext::OverloadValue<float,
      // 4>>>{},

      // we can also override default 'serialize' function by creating an
      // overloading for that type
      [](S& s, MyStruct& o) {
        s.value4b(o.f);
        s.container(o.v, 1000, [](S& s, int32_t& v) {
          s.ext4b(v, bitsery::ext::CompactValue{});
        });
      },
      // NOTE.
      // it is possible to provide "auto" as type parameter
      // this will allow you to override all default 'serialize' functions
      // but in this case it will not be called, because we have explicitly
      // provided overloads for all variant types
      // also note, that first parameter (serializer) is also "auto", this is
      // required, so that it would be least specialized case
      // otherwise it will not compile if you any ext::Overload* helper defined,
      // because it will have ambiguous definitions
      // (ext::OverLoad* defines (templated_type& s, concrete_type& o) and
      // lambda would be (concrete_type& s, templated_type& o))
      [](auto&, auto&) { assert(false); } });
}

// some helper types
using Buffer = eastl::vector<uint8_t>;
using OutputAdapter = bitsery::OutputBufferAdapter<Buffer>;
using InputAdapter = bitsery::InputBufferAdapter<Buffer>;

int
main()
{
  // set some random data
  MyVariant data{ MyTuple{ -7549, { { -451, 2, 968, 75, 4, 156, 49 }, 874.4f } } };
  MyVariant res{};

  // create buffer to store data
  Buffer buffer;
  // use quick serialization function,
  // it will use default configuration to setup all the nesessary steps
  // and serialize data to container
  auto writtenSize = bitsery::quickSerialization<OutputAdapter>(buffer, data);

  // same as serialization, but returns deserialization state as a pair
  // first = error code, second = is buffer was successfully read from begin to
  // the end.
  auto state = bitsery::quickDeserialization<InputAdapter>({ buffer.begin(), writtenSize }, res);

  assert(state.first == bitsery::ReaderError::NoError && state.second);
  assert(data == res);
}
#else
#if defined(_MSC_VER)
#pragma message(                                                               \
  "C++17 and /Zc:__cplusplus option is required to enable this example")
#else
#pragma message("C++17 is required to enable this example")
#endif
int
main()
{
  return -1;
}
#endif