/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "utils.h"

#include "class_linker-inl.h"
#include "common_runtime_test.h"
#include "mirror/array.h"
#include "mirror/array-inl.h"
#include "mirror/object-inl.h"
#include "mirror/object_array-inl.h"
#include "mirror/string.h"
#include "scoped_thread_state_change.h"
#include "handle_scope-inl.h"

#include <valgrind.h>

namespace art {

std::string PrettyArguments(const char* signature);
std::string PrettyReturnType(const char* signature);

class UtilsTest : public CommonRuntimeTest {};

TEST_F(UtilsTest, PrettyDescriptor_ArrayReferences) {
  EXPECT_EQ("java.lang.Class[]", PrettyDescriptor("[Ljava/lang/Class;"));
  EXPECT_EQ("java.lang.Class[][]", PrettyDescriptor("[[Ljava/lang/Class;"));
}

TEST_F(UtilsTest, PrettyDescriptor_ScalarReferences) {
  EXPECT_EQ("java.lang.String", PrettyDescriptor("Ljava.lang.String;"));
  EXPECT_EQ("java.lang.String", PrettyDescriptor("Ljava/lang/String;"));
}

TEST_F(UtilsTest, PrettyDescriptor_Primitive) {
  EXPECT_EQ("boolean", PrettyDescriptor(Primitive::kPrimBoolean));
  EXPECT_EQ("byte", PrettyDescriptor(Primitive::kPrimByte));
  EXPECT_EQ("char", PrettyDescriptor(Primitive::kPrimChar));
  EXPECT_EQ("short", PrettyDescriptor(Primitive::kPrimShort));
  EXPECT_EQ("int", PrettyDescriptor(Primitive::kPrimInt));
  EXPECT_EQ("float", PrettyDescriptor(Primitive::kPrimFloat));
  EXPECT_EQ("long", PrettyDescriptor(Primitive::kPrimLong));
  EXPECT_EQ("double", PrettyDescriptor(Primitive::kPrimDouble));
  EXPECT_EQ("void", PrettyDescriptor(Primitive::kPrimVoid));
}

TEST_F(UtilsTest, PrettyDescriptor_PrimitiveArrays) {
  EXPECT_EQ("boolean[]", PrettyDescriptor("[Z"));
  EXPECT_EQ("boolean[][]", PrettyDescriptor("[[Z"));
  EXPECT_EQ("byte[]", PrettyDescriptor("[B"));
  EXPECT_EQ("byte[][]", PrettyDescriptor("[[B"));
  EXPECT_EQ("char[]", PrettyDescriptor("[C"));
  EXPECT_EQ("char[][]", PrettyDescriptor("[[C"));
  EXPECT_EQ("double[]", PrettyDescriptor("[D"));
  EXPECT_EQ("double[][]", PrettyDescriptor("[[D"));
  EXPECT_EQ("float[]", PrettyDescriptor("[F"));
  EXPECT_EQ("float[][]", PrettyDescriptor("[[F"));
  EXPECT_EQ("int[]", PrettyDescriptor("[I"));
  EXPECT_EQ("int[][]", PrettyDescriptor("[[I"));
  EXPECT_EQ("long[]", PrettyDescriptor("[J"));
  EXPECT_EQ("long[][]", PrettyDescriptor("[[J"));
  EXPECT_EQ("short[]", PrettyDescriptor("[S"));
  EXPECT_EQ("short[][]", PrettyDescriptor("[[S"));
}

TEST_F(UtilsTest, PrettyDescriptor_PrimitiveScalars) {
  EXPECT_EQ("boolean", PrettyDescriptor("Z"));
  EXPECT_EQ("byte", PrettyDescriptor("B"));
  EXPECT_EQ("char", PrettyDescriptor("C"));
  EXPECT_EQ("double", PrettyDescriptor("D"));
  EXPECT_EQ("float", PrettyDescriptor("F"));
  EXPECT_EQ("int", PrettyDescriptor("I"));
  EXPECT_EQ("long", PrettyDescriptor("J"));
  EXPECT_EQ("short", PrettyDescriptor("S"));
}

TEST_F(UtilsTest, PrettyArguments) {
  EXPECT_EQ("()", PrettyArguments("()V"));
  EXPECT_EQ("(int)", PrettyArguments("(I)V"));
  EXPECT_EQ("(int, int)", PrettyArguments("(II)V"));
  EXPECT_EQ("(int, int, int[][])", PrettyArguments("(II[[I)V"));
  EXPECT_EQ("(int, int, int[][], java.lang.Poop)", PrettyArguments("(II[[ILjava/lang/Poop;)V"));
  EXPECT_EQ("(int, int, int[][], java.lang.Poop, java.lang.Poop[][])", PrettyArguments("(II[[ILjava/lang/Poop;[[Ljava/lang/Poop;)V"));
}

TEST_F(UtilsTest, PrettyReturnType) {
  EXPECT_EQ("void", PrettyReturnType("()V"));
  EXPECT_EQ("int", PrettyReturnType("()I"));
  EXPECT_EQ("int[][]", PrettyReturnType("()[[I"));
  EXPECT_EQ("java.lang.Poop", PrettyReturnType("()Ljava/lang/Poop;"));
  EXPECT_EQ("java.lang.Poop[][]", PrettyReturnType("()[[Ljava/lang/Poop;"));
}

TEST_F(UtilsTest, PrettyTypeOf) {
  ScopedObjectAccess soa(Thread::Current());
  EXPECT_EQ("null", PrettyTypeOf(nullptr));

  StackHandleScope<2> hs(soa.Self());
  Handle<mirror::String> s(hs.NewHandle(mirror::String::AllocFromModifiedUtf8(soa.Self(), "")));
  EXPECT_EQ("java.lang.String", PrettyTypeOf(s.Get()));

  Handle<mirror::ShortArray> a(hs.NewHandle(mirror::ShortArray::Alloc(soa.Self(), 2)));
  EXPECT_EQ("short[]", PrettyTypeOf(a.Get()));

  mirror::Class* c = class_linker_->FindSystemClass(soa.Self(), "[Ljava/lang/String;");
  ASSERT_TRUE(c != nullptr);
  mirror::Object* o = mirror::ObjectArray<mirror::String>::Alloc(soa.Self(), c, 0);
  EXPECT_EQ("java.lang.String[]", PrettyTypeOf(o));
  EXPECT_EQ("java.lang.Class<java.lang.String[]>", PrettyTypeOf(o->GetClass()));
}

TEST_F(UtilsTest, PrettyClass) {
  ScopedObjectAccess soa(Thread::Current());
  EXPECT_EQ("null", PrettyClass(nullptr));
  mirror::Class* c = class_linker_->FindSystemClass(soa.Self(), "[Ljava/lang/String;");
  ASSERT_TRUE(c != nullptr);
  mirror::Object* o = mirror::ObjectArray<mirror::String>::Alloc(soa.Self(), c, 0);
  EXPECT_EQ("java.lang.Class<java.lang.String[]>", PrettyClass(o->GetClass()));
}

TEST_F(UtilsTest, PrettyClassAndClassLoader) {
  ScopedObjectAccess soa(Thread::Current());
  EXPECT_EQ("null", PrettyClassAndClassLoader(nullptr));
  mirror::Class* c = class_linker_->FindSystemClass(soa.Self(), "[Ljava/lang/String;");
  ASSERT_TRUE(c != nullptr);
  mirror::Object* o = mirror::ObjectArray<mirror::String>::Alloc(soa.Self(), c, 0);
  EXPECT_EQ("java.lang.Class<java.lang.String[],null>", PrettyClassAndClassLoader(o->GetClass()));
}

TEST_F(UtilsTest, PrettyField) {
  ScopedObjectAccess soa(Thread::Current());
  EXPECT_EQ("null", PrettyField(nullptr));

  mirror::Class* java_lang_String = class_linker_->FindSystemClass(soa.Self(),
                                                                   "Ljava/lang/String;");

  ArtField* f;
  f = java_lang_String->FindDeclaredInstanceField("count", "I");
  EXPECT_EQ("int java.lang.String.count", PrettyField(f));
  EXPECT_EQ("java.lang.String.count", PrettyField(f, false));
}

TEST_F(UtilsTest, PrettySize) {
  EXPECT_EQ("1GB", PrettySize(1 * GB));
  EXPECT_EQ("2GB", PrettySize(2 * GB));
  if (sizeof(size_t) > sizeof(uint32_t)) {
    EXPECT_EQ("100GB", PrettySize(100 * GB));
  }
  EXPECT_EQ("1024KB", PrettySize(1 * MB));
  EXPECT_EQ("10MB", PrettySize(10 * MB));
  EXPECT_EQ("100MB", PrettySize(100 * MB));
  EXPECT_EQ("1024B", PrettySize(1 * KB));
  EXPECT_EQ("10KB", PrettySize(10 * KB));
  EXPECT_EQ("100KB", PrettySize(100 * KB));
  EXPECT_EQ("0B", PrettySize(0));
  EXPECT_EQ("1B", PrettySize(1));
  EXPECT_EQ("10B", PrettySize(10));
  EXPECT_EQ("100B", PrettySize(100));
  EXPECT_EQ("512B", PrettySize(512));
}

TEST_F(UtilsTest, PrettyDuration) {
  const uint64_t one_sec = 1000000000;
  const uint64_t one_ms  = 1000000;
  const uint64_t one_us  = 1000;

  EXPECT_EQ("1s", PrettyDuration(1 * one_sec));
  EXPECT_EQ("10s", PrettyDuration(10 * one_sec));
  EXPECT_EQ("100s", PrettyDuration(100 * one_sec));
  EXPECT_EQ("1.001s", PrettyDuration(1 * one_sec + one_ms));
  EXPECT_EQ("1.000001s", PrettyDuration(1 * one_sec + one_us, 6));
  EXPECT_EQ("1.000000001s", PrettyDuration(1 * one_sec + 1, 9));
  EXPECT_EQ("1.000s", PrettyDuration(1 * one_sec + one_us, 3));

  EXPECT_EQ("1ms", PrettyDuration(1 * one_ms));
  EXPECT_EQ("10ms", PrettyDuration(10 * one_ms));
  EXPECT_EQ("100ms", PrettyDuration(100 * one_ms));
  EXPECT_EQ("1.001ms", PrettyDuration(1 * one_ms + one_us));
  EXPECT_EQ("1.000001ms", PrettyDuration(1 * one_ms + 1, 6));

  EXPECT_EQ("1us", PrettyDuration(1 * one_us));
  EXPECT_EQ("10us", PrettyDuration(10 * one_us));
  EXPECT_EQ("100us", PrettyDuration(100 * one_us));
  EXPECT_EQ("1.001us", PrettyDuration(1 * one_us + 1));

  EXPECT_EQ("1ns", PrettyDuration(1));
  EXPECT_EQ("10ns", PrettyDuration(10));
  EXPECT_EQ("100ns", PrettyDuration(100));
}

TEST_F(UtilsTest, MangleForJni) {
  ScopedObjectAccess soa(Thread::Current());
  EXPECT_EQ("hello_00024world", MangleForJni("hello$world"));
  EXPECT_EQ("hello_000a9world", MangleForJni("hello\xc2\xa9world"));
  EXPECT_EQ("hello_1world", MangleForJni("hello_world"));
  EXPECT_EQ("Ljava_lang_String_2", MangleForJni("Ljava/lang/String;"));
  EXPECT_EQ("_3C", MangleForJni("[C"));
}

TEST_F(UtilsTest, JniShortName_JniLongName) {
  ScopedObjectAccess soa(Thread::Current());
  mirror::Class* c = class_linker_->FindSystemClass(soa.Self(), "Ljava/lang/String;");
  ASSERT_TRUE(c != nullptr);
  mirror::ArtMethod* m;

  m = c->FindVirtualMethod("charAt", "(I)C");
  ASSERT_TRUE(m != nullptr);
  EXPECT_EQ("Java_java_lang_String_charAt", JniShortName(m));
  EXPECT_EQ("Java_java_lang_String_charAt__I", JniLongName(m));

  m = c->FindVirtualMethod("indexOf", "(Ljava/lang/String;I)I");
  ASSERT_TRUE(m != nullptr);
  EXPECT_EQ("Java_java_lang_String_indexOf", JniShortName(m));
  EXPECT_EQ("Java_java_lang_String_indexOf__Ljava_lang_String_2I", JniLongName(m));

  m = c->FindDirectMethod("copyValueOf", "([CII)Ljava/lang/String;");
  ASSERT_TRUE(m != nullptr);
  EXPECT_EQ("Java_java_lang_String_copyValueOf", JniShortName(m));
  EXPECT_EQ("Java_java_lang_String_copyValueOf___3CII", JniLongName(m));
}

TEST_F(UtilsTest, Split) {
  std::vector<std::string> actual;
  std::vector<std::string> expected;

  expected.clear();

  actual.clear();
  Split("", ':', &actual);
  EXPECT_EQ(expected, actual);

  actual.clear();
  Split(":", ':', &actual);
  EXPECT_EQ(expected, actual);

  expected.clear();
  expected.push_back("foo");

  actual.clear();
  Split(":foo", ':', &actual);
  EXPECT_EQ(expected, actual);

  actual.clear();
  Split("foo:", ':', &actual);
  EXPECT_EQ(expected, actual);

  actual.clear();
  Split(":foo:", ':', &actual);
  EXPECT_EQ(expected, actual);

  expected.push_back("bar");

  actual.clear();
  Split("foo:bar", ':', &actual);
  EXPECT_EQ(expected, actual);

  actual.clear();
  Split(":foo:bar", ':', &actual);
  EXPECT_EQ(expected, actual);

  actual.clear();
  Split("foo:bar:", ':', &actual);
  EXPECT_EQ(expected, actual);

  actual.clear();
  Split(":foo:bar:", ':', &actual);
  EXPECT_EQ(expected, actual);

  expected.push_back("baz");

  actual.clear();
  Split("foo:bar:baz", ':', &actual);
  EXPECT_EQ(expected, actual);

  actual.clear();
  Split(":foo:bar:baz", ':', &actual);
  EXPECT_EQ(expected, actual);

  actual.clear();
  Split("foo:bar:baz:", ':', &actual);
  EXPECT_EQ(expected, actual);

  actual.clear();
  Split(":foo:bar:baz:", ':', &actual);
  EXPECT_EQ(expected, actual);
}

TEST_F(UtilsTest, Join) {
  std::vector<std::string> strings;

  strings.clear();
  EXPECT_EQ("", Join(strings, ':'));

  strings.clear();
  strings.push_back("foo");
  EXPECT_EQ("foo", Join(strings, ':'));

  strings.clear();
  strings.push_back("");
  strings.push_back("foo");
  EXPECT_EQ(":foo", Join(strings, ':'));

  strings.clear();
  strings.push_back("foo");
  strings.push_back("");
  EXPECT_EQ("foo:", Join(strings, ':'));

  strings.clear();
  strings.push_back("");
  strings.push_back("foo");
  strings.push_back("");
  EXPECT_EQ(":foo:", Join(strings, ':'));

  strings.clear();
  strings.push_back("foo");
  strings.push_back("bar");
  EXPECT_EQ("foo:bar", Join(strings, ':'));

  strings.clear();
  strings.push_back("foo");
  strings.push_back("bar");
  strings.push_back("baz");
  EXPECT_EQ("foo:bar:baz", Join(strings, ':'));
}

TEST_F(UtilsTest, StartsWith) {
  EXPECT_FALSE(StartsWith("foo", "bar"));
  EXPECT_TRUE(StartsWith("foo", "foo"));
  EXPECT_TRUE(StartsWith("food", "foo"));
  EXPECT_FALSE(StartsWith("fo", "foo"));
}

TEST_F(UtilsTest, EndsWith) {
  EXPECT_FALSE(EndsWith("foo", "bar"));
  EXPECT_TRUE(EndsWith("foo", "foo"));
  EXPECT_TRUE(EndsWith("foofoo", "foo"));
  EXPECT_FALSE(EndsWith("oo", "foo"));
}

TEST_F(UtilsTest, GetDalvikCacheFilenameOrDie) {
  EXPECT_STREQ("/foo/system@app@Foo.apk@classes.dex",
               GetDalvikCacheFilenameOrDie("/system/app/Foo.apk", "/foo").c_str());

  EXPECT_STREQ("/foo/data@app@foo-1.apk@classes.dex",
               GetDalvikCacheFilenameOrDie("/data/app/foo-1.apk", "/foo").c_str());
  EXPECT_STREQ("/foo/system@framework@core.jar@classes.dex",
               GetDalvikCacheFilenameOrDie("/system/framework/core.jar", "/foo").c_str());
  EXPECT_STREQ("/foo/system@framework@boot.art",
               GetDalvikCacheFilenameOrDie("/system/framework/boot.art", "/foo").c_str());
  EXPECT_STREQ("/foo/system@framework@boot.oat",
               GetDalvikCacheFilenameOrDie("/system/framework/boot.oat", "/foo").c_str());
}

TEST_F(UtilsTest, GetDalvikCache) {
  EXPECT_STREQ("", GetDalvikCache("should-not-exist123", false).c_str());

  EXPECT_STREQ((android_data_ + "/dalvik-cache/.").c_str(), GetDalvikCache(".", false).c_str());
  EXPECT_STREQ((android_data_ + "/dalvik-cache/should-not-be-there").c_str(),
               GetDalvikCache("should-not-be-there", true).c_str());
}


TEST_F(UtilsTest, GetSystemImageFilename) {
  EXPECT_STREQ("/system/framework/arm/boot.art",
               GetSystemImageFilename("/system/framework/boot.art", kArm).c_str());
}

TEST_F(UtilsTest, ExecSuccess) {
  std::vector<std::string> command;
  if (kIsTargetBuild) {
    std::string android_root(GetAndroidRoot());
    command.push_back(android_root + "/bin/id");
  } else {
    command.push_back("/usr/bin/id");
  }
  std::string error_msg;
  if (RUNNING_ON_VALGRIND == 0) {
    // Running on valgrind fails due to some memory that leaks in thread alternate signal stacks.
    EXPECT_TRUE(Exec(command, &error_msg));
  }
  EXPECT_EQ(0U, error_msg.size()) << error_msg;
}

TEST_F(UtilsTest, ExecError) {
  // This will lead to error messages in the log.
  ScopedLogSeverity sls(LogSeverity::FATAL);

  std::vector<std::string> command;
  command.push_back("bogus");
  std::string error_msg;
  if (RUNNING_ON_VALGRIND == 0) {
    // Running on valgrind fails due to some memory that leaks in thread alternate signal stacks.
    EXPECT_FALSE(Exec(command, &error_msg));
    EXPECT_NE(0U, error_msg.size());
  }
}

TEST_F(UtilsTest, RoundUpToPowerOfTwo) {
  // Tests the constexpr variant since all the parameters are constexpr
  EXPECT_EQ(0, RoundUpToPowerOfTwo(0));
  EXPECT_EQ(1, RoundUpToPowerOfTwo(1));
  EXPECT_EQ(2, RoundUpToPowerOfTwo(2));
  EXPECT_EQ(4, RoundUpToPowerOfTwo(3));
  EXPECT_EQ(8, RoundUpToPowerOfTwo(7));

  EXPECT_EQ(0b10000L, RoundUpToPowerOfTwo(0b01101L));
  EXPECT_EQ(1ULL << 63, RoundUpToPowerOfTwo(1ULL << 62 | 1ULL));
}

TEST_F(UtilsTest, MostSignificantBit) {
  EXPECT_EQ(-1, MostSignificantBit(0));
  EXPECT_EQ(0, MostSignificantBit(1));
  EXPECT_EQ(31, MostSignificantBit(~static_cast<uint32_t>(0)));
  EXPECT_EQ(2, MostSignificantBit(0b110));
  EXPECT_EQ(2, MostSignificantBit(0b100));
}

TEST_F(UtilsTest, MinimumBitsToStore) {
  EXPECT_EQ(0u, MinimumBitsToStore(0));
  EXPECT_EQ(1u, MinimumBitsToStore(1));
  EXPECT_EQ(2u, MinimumBitsToStore(0b10));
  EXPECT_EQ(2u, MinimumBitsToStore(0b11));
  EXPECT_EQ(3u, MinimumBitsToStore(0b100));
  EXPECT_EQ(3u, MinimumBitsToStore(0b110));
  EXPECT_EQ(3u, MinimumBitsToStore(0b101));
  EXPECT_EQ(8u, MinimumBitsToStore(0xFF));
  EXPECT_EQ(32u, MinimumBitsToStore(~static_cast<uint32_t>(0)));
}

static constexpr int64_t INT_MIN_minus1 = static_cast<int64_t>(INT_MIN) - 1;
static constexpr int64_t INT_MAX_plus1 = static_cast<int64_t>(INT_MAX) + 1;
static constexpr int64_t UINT_MAX_plus1 = static_cast<int64_t>(UINT_MAX) + 1;

TEST_F(UtilsTest, IsInt) {
  EXPECT_FALSE(IsInt(1, -2));
  EXPECT_TRUE(IsInt(1, -1));
  EXPECT_TRUE(IsInt(1, 0));
  EXPECT_FALSE(IsInt(1, 1));

  EXPECT_FALSE(IsInt(4, -9));
  EXPECT_TRUE(IsInt(4, -8));
  EXPECT_TRUE(IsInt(4, 7));
  EXPECT_FALSE(IsInt(4, 8));

  EXPECT_FALSE(IsInt(32, INT_MIN_minus1));
  EXPECT_TRUE(IsInt(32, INT_MIN));
  EXPECT_TRUE(IsInt(32, INT_MAX));
  EXPECT_FALSE(IsInt(32, INT_MAX_plus1));
}

TEST_F(UtilsTest, IsInt_Static) {
  EXPECT_FALSE(IsInt<1>(-2));
  EXPECT_TRUE(IsInt<1>(-1));
  EXPECT_TRUE(IsInt<1>(0));
  EXPECT_FALSE(IsInt<1>(1));

  EXPECT_FALSE(IsInt<4>(-9));
  EXPECT_TRUE(IsInt<4>(-8));
  EXPECT_TRUE(IsInt<4>(7));
  EXPECT_FALSE(IsInt<4>(8));

  EXPECT_FALSE(IsInt<32>(INT_MIN_minus1));
  EXPECT_TRUE(IsInt<32>(INT_MIN));
  EXPECT_TRUE(IsInt<32>(INT_MAX));
  EXPECT_FALSE(IsInt<32>(INT_MAX_plus1));
}

TEST_F(UtilsTest, IsUint) {
  EXPECT_FALSE(IsUint<1>(-1));
  EXPECT_TRUE(IsUint<1>(0));
  EXPECT_TRUE(IsUint<1>(1));
  EXPECT_FALSE(IsUint<1>(2));

  EXPECT_FALSE(IsUint<4>(-1));
  EXPECT_TRUE(IsUint<4>(0));
  EXPECT_TRUE(IsUint<4>(15));
  EXPECT_FALSE(IsUint<4>(16));

  EXPECT_FALSE(IsUint<32>(-1));
  EXPECT_TRUE(IsUint<32>(0));
  EXPECT_TRUE(IsUint<32>(UINT_MAX));
  EXPECT_FALSE(IsUint<32>(UINT_MAX_plus1));
}

TEST_F(UtilsTest, IsAbsoluteUint) {
  EXPECT_FALSE(IsAbsoluteUint<1>(-2));
  EXPECT_TRUE(IsAbsoluteUint<1>(-1));
  EXPECT_TRUE(IsAbsoluteUint<32>(0));
  EXPECT_TRUE(IsAbsoluteUint<1>(1));
  EXPECT_FALSE(IsAbsoluteUint<1>(2));

  EXPECT_FALSE(IsAbsoluteUint<4>(-16));
  EXPECT_TRUE(IsAbsoluteUint<4>(-15));
  EXPECT_TRUE(IsAbsoluteUint<32>(0));
  EXPECT_TRUE(IsAbsoluteUint<4>(15));
  EXPECT_FALSE(IsAbsoluteUint<4>(16));

  EXPECT_FALSE(IsAbsoluteUint<32>(-UINT_MAX_plus1));
  EXPECT_TRUE(IsAbsoluteUint<32>(-UINT_MAX));
  EXPECT_TRUE(IsAbsoluteUint<32>(0));
  EXPECT_TRUE(IsAbsoluteUint<32>(UINT_MAX));
  EXPECT_FALSE(IsAbsoluteUint<32>(UINT_MAX_plus1));
}

TEST_F(UtilsTest, TestSleep) {
  auto start = NanoTime();
  NanoSleep(MsToNs(1500));
  EXPECT_GT(NanoTime() - start, MsToNs(1000));
}

TEST_F(UtilsTest, IsValidDescriptor) {
  std::vector<uint8_t> descriptor(
      { 'L', 'a', '/', 'b', '$', 0xed, 0xa0, 0x80, 0xed, 0xb0, 0x80, ';', 0x00 });
  EXPECT_TRUE(IsValidDescriptor(reinterpret_cast<char*>(&descriptor[0])));

  std::vector<uint8_t> unpaired_surrogate(
      { 'L', 'a', '/', 'b', '$', 0xed, 0xa0, 0x80, ';', 0x00 });
  EXPECT_FALSE(IsValidDescriptor(reinterpret_cast<char*>(&unpaired_surrogate[0])));

  std::vector<uint8_t> unpaired_surrogate_at_end(
      { 'L', 'a', '/', 'b', '$', 0xed, 0xa0, 0x80, 0x00 });
  EXPECT_FALSE(IsValidDescriptor(reinterpret_cast<char*>(&unpaired_surrogate_at_end[0])));

  std::vector<uint8_t> invalid_surrogate(
      { 'L', 'a', '/', 'b', '$', 0xed, 0xb0, 0x80, ';', 0x00 });
  EXPECT_FALSE(IsValidDescriptor(reinterpret_cast<char*>(&invalid_surrogate[0])));

  std::vector<uint8_t> unpaired_surrogate_with_multibyte_sequence(
      { 'L', 'a', '/', 'b', '$', 0xed, 0xb0, 0x80, 0xf0, 0x9f, 0x8f, 0xa0, ';', 0x00 });
  EXPECT_FALSE(
      IsValidDescriptor(reinterpret_cast<char*>(&unpaired_surrogate_with_multibyte_sequence[0])));
}

}  // namespace art
