#include <bitsery/adapter/buffer.h>
#include <bitsery/bitsery.h>
// to use brief syntax always include this header
#include <bitsery/brief_syntax.h>
// we also need additional traits to work with container types,
// instead of including <bitsery/traits/vector.h> for vector traits, now we also
// need traits to work with brief_syntax types. so include everything from
// <bitsery/brief_syntax/...> instead of <bitsery/traits/...> otherwise we'll
// get static assert error, saying to define serialize function.
#include <bitsery/brief_syntax/vector.h>

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

enum class MyEnum : uint16_t
{
  V1,
  V2,
  V3
};
struct MyStruct
{
  uint32_t i;
  MyEnum e;
  eastl::vector<float> fs;

  // define serialize function as usual
  template<typename S>
  void serialize(S& s)
  {
    // now we can use brief syntax with
    s(i, e, fs);
  }
};

// some helper types
using Buffer = eastl::vector<uint8_t>;
using OutputAdapter = bitsery::OutputBufferAdapter<Buffer>;
using InputAdapter = bitsery::InputBufferAdapter<Buffer>;

int
main()
{
  // set some random data
  MyStruct data{ 8941, MyEnum::V2, { 15.0f, -8.5f, 0.045f } };
  MyStruct res{};

  // serialization, deserialization flow is unchanged as in basic usage
  Buffer buffer;
  auto writtenSize = bitsery::quickSerialization<OutputAdapter>(buffer, data);

  auto state = bitsery::quickDeserialization<InputAdapter>({ buffer.begin(), writtenSize }, res);

  assert(state.first == bitsery::ReaderError::NoError && state.second);
  assert(data.fs == res.fs && data.i == res.i && data.e == res.e);
}
