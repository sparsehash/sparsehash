// Copyright (c) 2006, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// ----
// Author: Matt Austern

#include <google/sparsehash/config.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <google/type_traits.h>

using STL_NAMESPACE::string;
using STL_NAMESPACE::vector;
using STL_NAMESPACE::pair;

#define ASSERT_TRUE(cond)  do {                 \
  if ( !(cond) ) {                              \
    printf("Test failed: " #cond "\n");         \
    exit(1);                                    \
  }                                             \
} while (0)

#define ASSERT_FALSE(cond)  ASSERT_TRUE(!(cond))


// A user-defined POD type.
struct A {
  int n_;
};

// A user-defined non-POD type with a trivial copy constructor.
class B {
 public:
  B(int n) : n_(n) { }
 private:
  int n_;
};

// Another user-defined non-POD type with a trivial copy constructor.
// We will explicitly declare C to have a trivial copy constructor
// by specializing has_trivial_copy.
class C {
 public:
  C(int n) : n_(n) { }
 private:
  int n_;
};

_START_GOOGLE_NAMESPACE_
template<> struct has_trivial_copy<C> : true_type { };
_END_GOOGLE_NAMESPACE_

// Another user-defined non-POD type with a trivial assignment operator.
// We will explicitly declare C to have a trivial assignment operator
// by specializing has_trivial_assign.
class D {
 public:
  D(int n) : n_(n) { }
 private:
  int n_;
};

_START_GOOGLE_NAMESPACE_
template<> struct has_trivial_assign<D> : true_type { };
_END_GOOGLE_NAMESPACE_

namespace {

// type_equals_ is a template type comparator, similar to Loki IsSameType.
// type_equals_<A, B>::value is true iff "A" is the same type as "B".
template<typename A, typename B>
struct type_equals_ : public GOOGLE_NAMESPACE::false_type {
};

template<typename A>
struct type_equals_<A, A> : public GOOGLE_NAMESPACE::true_type {
};

// This assertion produces errors like "error: invalid use of
// undefined type 'struct <unnamed>::AssertTypesEq<const int, int>'"
// when it fails.
template<typename T, typename U> struct AssertTypesEq;
template<typename T> struct AssertTypesEq<T, T> {};
#define COMPILE_ASSERT_TYPES_EQ(T, U) AssertTypesEq<T, U>()

class TypeTraitsTest {
 public:
  static void TestIsInteger() {
    // Verify that is_integral is true for all integer types.
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_integral<bool>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_integral<char>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_integral<unsigned char>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_integral<signed char>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_integral<wchar_t>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_integral<int>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_integral<unsigned int>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_integral<short>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_integral<unsigned short>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_integral<long>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_integral<unsigned long>::value);

    // Verify that is_integral is false for a few non-integer types.
    ASSERT_FALSE(GOOGLE_NAMESPACE::is_integral<void>::value);
    ASSERT_FALSE(GOOGLE_NAMESPACE::is_integral<float>::value);
    ASSERT_FALSE(GOOGLE_NAMESPACE::is_integral<string>::value);
    ASSERT_FALSE(GOOGLE_NAMESPACE::is_integral<int*>::value);
    ASSERT_FALSE(GOOGLE_NAMESPACE::is_integral<A>::value);
    ASSERT_FALSE((GOOGLE_NAMESPACE::is_integral<pair<int, int> >::value));
  }

  static void TestIsFloating() {
    // Verify that is_floating_point is true for all floating-point types.
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_floating_point<float>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_floating_point<double>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_floating_point<long double>::value);

    // Verify that is_floating_point is false for a few non-float types.
    ASSERT_FALSE(GOOGLE_NAMESPACE::is_floating_point<void>::value);
    ASSERT_FALSE(GOOGLE_NAMESPACE::is_floating_point<long>::value);
    ASSERT_FALSE(GOOGLE_NAMESPACE::is_floating_point<string>::value);
    ASSERT_FALSE(GOOGLE_NAMESPACE::is_floating_point<float*>::value);
    ASSERT_FALSE(GOOGLE_NAMESPACE::is_floating_point<A>::value);
    ASSERT_FALSE((GOOGLE_NAMESPACE::is_floating_point<pair<int, int> >::value));
  }

  static void TestIsPod() {
    // Verify that arithmetic types and pointers are marked as PODs.
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_pod<bool>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_pod<char>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_pod<unsigned char>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_pod<signed char>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_pod<wchar_t>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_pod<int>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_pod<unsigned int>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_pod<short>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_pod<unsigned short>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_pod<long>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_pod<unsigned long>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_pod<float>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_pod<double>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_pod<long double>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_pod<string*>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_pod<A*>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_pod<const B*>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::is_pod<C**>::value);

    // Verify that some non-POD types are not marked as PODs.
    ASSERT_FALSE(GOOGLE_NAMESPACE::is_pod<void>::value);
    ASSERT_FALSE(GOOGLE_NAMESPACE::is_pod<string>::value);
    ASSERT_FALSE((GOOGLE_NAMESPACE::is_pod<pair<int, int> >::value));
    ASSERT_FALSE(GOOGLE_NAMESPACE::is_pod<A>::value);
    ASSERT_FALSE(GOOGLE_NAMESPACE::is_pod<B>::value);
    ASSERT_FALSE(GOOGLE_NAMESPACE::is_pod<C>::value);
  }

  static void TestHasTrivialCopy() {
    // Verify that arithmetic types and pointers have trivial copy
    // constructors.
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_copy<bool>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_copy<char>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_copy<unsigned char>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_copy<signed char>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_copy<wchar_t>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_copy<int>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_copy<unsigned int>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_copy<short>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_copy<unsigned short>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_copy<long>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_copy<unsigned long>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_copy<float>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_copy<double>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_copy<long double>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_copy<string*>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_copy<A*>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_copy<const B*>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_copy<C**>::value);

    // Verify that pairs and arrays of such types have trivial
    // copy constructors.
    typedef int int10[10];
    ASSERT_TRUE((GOOGLE_NAMESPACE::has_trivial_copy<pair<int, char*> >::value));
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_copy<int10>::value);

    // Verify that types without trivial copy constructors are
    // correctly marked as such.
    ASSERT_FALSE(GOOGLE_NAMESPACE::has_trivial_copy<string>::value);
    ASSERT_FALSE(GOOGLE_NAMESPACE::has_trivial_copy<vector<int> >::value);

    // Verify that pairs of types without trivial copy constructors
    // are not marked as trivial.
    ASSERT_FALSE((GOOGLE_NAMESPACE::has_trivial_copy<pair<int, string> >::value));
    ASSERT_FALSE((GOOGLE_NAMESPACE::has_trivial_copy<pair<string, int> >::value));

    // Verify that C, which we have declared to have a trivial
    // copy constructor, is correctly marked as such.
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_copy<C>::value);
  }

  static void TestHasTrivialAssign() {
    // Verify that arithmetic types and pointers have trivial assignment
    // operators.
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_assign<bool>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_assign<char>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_assign<unsigned char>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_assign<signed char>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_assign<wchar_t>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_assign<int>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_assign<unsigned int>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_assign<short>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_assign<unsigned short>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_assign<long>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_assign<unsigned long>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_assign<float>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_assign<double>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_assign<long double>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_assign<string*>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_assign<A*>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_assign<const B*>::value);
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_assign<C**>::value);

    // Verify that pairs and arrays of such types have trivial
    // assignment operators.
    typedef int int10[10];
    ASSERT_TRUE((GOOGLE_NAMESPACE::has_trivial_assign<pair<int, char*> >::value));
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_assign<int10>::value);

    // Verify that pairs of types without trivial assignment operators
    // are not marked as trivial.
    ASSERT_FALSE((GOOGLE_NAMESPACE::has_trivial_assign<pair<int, string> >::value));
    ASSERT_FALSE((GOOGLE_NAMESPACE::has_trivial_assign<pair<string, int> >::value));

    // Verify that types without trivial assignment operators are
    // correctly marked as such.
    ASSERT_FALSE(GOOGLE_NAMESPACE::has_trivial_assign<string>::value);
    ASSERT_FALSE(GOOGLE_NAMESPACE::has_trivial_assign<vector<int> >::value);

    // Verify that D, which we have declared to have a trivial
    // assignment operator, is correctly marked as such.
    ASSERT_TRUE(GOOGLE_NAMESPACE::has_trivial_assign<D>::value);
  }

  // Tests remove_pointer.
  static void TestRemovePointer() {
    COMPILE_ASSERT_TYPES_EQ(int, GOOGLE_NAMESPACE::remove_pointer<int>::type);
    COMPILE_ASSERT_TYPES_EQ(int, GOOGLE_NAMESPACE::remove_pointer<int*>::type);
    COMPILE_ASSERT_TYPES_EQ(const int, GOOGLE_NAMESPACE::remove_pointer<const int*>::type);
    COMPILE_ASSERT_TYPES_EQ(int, GOOGLE_NAMESPACE::remove_pointer<int* const>::type);
    COMPILE_ASSERT_TYPES_EQ(int, GOOGLE_NAMESPACE::remove_pointer<int* volatile>::type);
  }

};  // end class TypeTraitsTest

}   // end anonymous namespace


int main(int argc, char **argv) {
  TypeTraitsTest::TestIsInteger();
  TypeTraitsTest::TestIsFloating();
  TypeTraitsTest::TestIsPod();
  TypeTraitsTest::TestHasTrivialCopy();
  TypeTraitsTest::TestHasTrivialAssign();
  TypeTraitsTest::TestRemovePointer();

  printf("PASS\n");
  return 0;
}
