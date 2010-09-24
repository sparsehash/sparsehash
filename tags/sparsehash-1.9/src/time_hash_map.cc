// Copyright (c) 2005, Google Inc.
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

// ---
// Authors: Sanjay Ghemawat and Craig Silverstein
//
// Time various hash map implementations
//
// Consider doing the following to get good numbers:
//
// 1. Run the tests on a machine with no X service. Make sure no other
//    processes are running.
// 2. Minimize compiled-code differences. Compare results from the same
//    binary, if possible, instead of comparing results from two different
//    binaries.
//
// See PERFORMANCE for the output of one example run.

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include <time.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>      // for uname()
#endif
#ifdef HAVE_WINDOWS_H
#include <Windows.h>          // for GetTickCount()
#endif
}

// The functions that we call on each map, that differ for different types.
// By default each is a noop, but we redefine them for types that need them.

#include <map>
#include HASH_MAP_H
#include <google/type_traits.h>

#include <google/sparse_hash_map>
using GOOGLE_NAMESPACE::sparse_hash_map;

#include <google/dense_hash_map>
using GOOGLE_NAMESPACE::dense_hash_map;


static bool FLAGS_test_sparse_hash_map = true;
static bool FLAGS_test_dense_hash_map = true;
static bool FLAGS_test_hash_map = true;
static bool FLAGS_test_map = true;

static bool FLAGS_test_4_bytes = true;
static bool FLAGS_test_16_bytes = true;
static bool FLAGS_test_256_bytes = true;

static const int kDefaultIters = 10000000;

// Normally I don't like non-const references, but using them here ensures
// the inlined code ends up as efficient as possible.

// These are operations that are supported on some hash impls but not others
template<class MapType> inline void SET_DELETED_KEY(MapType&, int /*key*/) {}
template<class MapType> inline void SET_EMPTY_KEY(MapType&, int /*key*/) {}
template<class MapType> inline void RESIZE(MapType&, int /*iters*/) {}

template<class K, class V, class H, class E, class A>
inline void SET_DELETED_KEY(sparse_hash_map<K,V,H,E,A>& m, int key) {
  m.set_deleted_key(key);
}
template<class K, class V, class H, class E, class A>
inline void SET_DELETED_KEY(dense_hash_map<K,V,H,E,A>& m, int key) {
  m.set_deleted_key(key);
}

template<class K, class V, class H, class E, class A>
inline void SET_EMPTY_KEY(dense_hash_map<K,V,H,E,A>& m, int key) {
  m.set_empty_key(key);
}

template<class K, class V, class H, class E, class A>
inline void RESIZE(sparse_hash_map<K,V,H,E,A>& m, int iters) {
  m.resize(iters);
}
template<class K, class V, class H, class E, class A>
inline void RESIZE(dense_hash_map<K,V,H,E,A>& m, int iters) {
  m.resize(iters);
}
#if defined(HAVE_UNORDERED_MAP)
template<class K, class V, class H, class E, class A>
inline void RESIZE(HASH_NAMESPACE::unordered_map<K,V,H,E,A>& m, int iters) {
  m.rehash(iters);   // the tr1 name for resize()
}
#elif defined(_MSC_VER)
// MSVC/Dinkumware hash impl has fewer template args, and no resize support.
template<class K, class V, class H, class A>
inline void RESIZE(HASH_NAMESPACE::hash_map<K,V,H,A>& m, int iters) {
}
#elif defined(HAVE_HASH_MAP)
template<class K, class V, class H, class E, class A>
inline void RESIZE(HASH_NAMESPACE::hash_map<K,V,H,E,A>& m, int iters) {
  m.resize(iters);
}
#endif  // HAVE_HASH_MAP

// Returns the number of hashes that have been done since the last
// call to NumHashesSinceLastCall().  This is shared across all
// HashObject instances, which isn't super-OO, but avoids two issues:
// (1) making HashObject bigger than it ought to be (this is very
// important for our testing), and (2) having to pass around
// HashObject objects everywhere, which is annoying.
static int g_num_hashes;
static int g_num_copies;

int NumHashesSinceLastCall() {
  int retval = g_num_hashes;
  g_num_hashes = 0;
  return retval;
}
int NumCopiesSinceLastCall() {
  int retval = g_num_copies;
  g_num_copies = 0;
  return retval;
}

/*
 * These are the objects we hash.  Size is the size of the object
 * (must be > sizeof(int).  Hashsize is how many of these bytes we
 * use when hashing (must be > sizeof(int) and < Size).
 */
template<int Size, int Hashsize> class HashObject {
 public:
  typedef HashObject<Size, Hashsize> class_type;
  HashObject() {}
  HashObject(int i) : i_(i) {
    memset(buffer_, i & 255, sizeof(buffer_));   // a "random" char
  }
  HashObject(const HashObject& that) {
    operator=(that);
  }
  void operator=(const HashObject& that) {
    g_num_copies++;
    this->i_ = that.i_;
    memcpy(this->buffer_, that.buffer_, sizeof(this->buffer_));
  }

  size_t Hash() const {
    g_num_hashes++;
    int hashval = i_;
    for (size_t i = 0; i < Hashsize - sizeof(i_); ++i) {
      hashval += buffer_[i];
    }
    return SPARSEHASH_HASH<int>()(hashval);   // defined in sparseconfig.h
  }

  bool operator==(const class_type& that) const { return this->i_ == that.i_; }
  bool operator< (const class_type& that) const { return this->i_ < that.i_; }
  bool operator<=(const class_type& that) const { return this->i_ <= that.i_; }

 private:
  int i_;        // the key used for hashing
  char buffer_[Size - sizeof(int)];
};

// A specialization for the case sizeof(buffer_) == 0
template<> class HashObject<sizeof(int), sizeof(int)> {
 public:
  typedef HashObject<sizeof(int), sizeof(int)> class_type;
  HashObject() {}
  HashObject(int i) : i_(i) {}
  HashObject(const HashObject& that) {
    operator=(that);
  }
  void operator=(const HashObject& that) {
    g_num_copies++;
    this->i_ = that.i_;
  }

  size_t Hash() const {
    g_num_hashes++;
    return SPARSEHASH_HASH<int>()(i_);
  }

  bool operator==(const class_type& that) const { return this->i_ == that.i_; }
  bool operator< (const class_type& that) const { return this->i_ < that.i_; }
  bool operator<=(const class_type& that) const { return this->i_ <= that.i_; }

 private:
  int i_;        // the key used for hashing
};

// Let the hashtable implementations know it can use an optimized memcpy,
// because the compiler defines both the destructor and copy constructor.

_START_GOOGLE_NAMESPACE_
template<int Size, int Hashsize>
struct has_trivial_copy< HashObject<Size, Hashsize> > : true_type { };

template<int Size, int Hashsize>
struct has_trivial_destructor< HashObject<Size, Hashsize> > : true_type { };
_END_GOOGLE_NAMESPACE_

class HashFn {
 public:
  template<int Size, int Hashsize>
  size_t operator()(const HashObject<Size,Hashsize>& obj) const {
    return obj.Hash();
  }
  // For windows
  template<int Size, int Hashsize>
  bool operator()(const HashObject<Size,Hashsize>& a,
                  const HashObject<Size,Hashsize>& b) const {
    return a < b;
  }
  // These two public members are required by msvc.  4 and 8 are defaults.
  static const size_t bucket_size = 4;
  static const size_t min_buckets = 8;
};

/*
 * Measure resource usage.
 */

class Rusage {
 public:
  /* Start collecting usage */
  Rusage() { Reset(); }

  /* Reset collection */
  void Reset();

  /* Show usage */
  double UserTime();

 private:
#if defined HAVE_SYS_RESOURCE_H
  struct rusage start;
#elif defined HAVE_WINDOWS_H
  long long int start;
#else
  time_t start_time_t;
#endif
};

inline void Rusage::Reset() {
#if defined HAVE_SYS_RESOURCE_H
  getrusage(RUSAGE_SELF, &start);
#elif defined HAVE_WINDOWS_H
  start = GetTickCount();
#else
  time(&start_time_t);
#endif
}

inline double Rusage::UserTime() {
#if defined HAVE_SYS_RESOURCE_H
  struct rusage u;

  getrusage(RUSAGE_SELF, &u);

  struct timeval result;
  result.tv_sec  = u.ru_utime.tv_sec  - start.ru_utime.tv_sec;
  result.tv_usec = u.ru_utime.tv_usec - start.ru_utime.tv_usec;

  return double(result.tv_sec) + double(result.tv_usec) / 1000000.0;
#elif defined HAVE_WINDOWS_H
  return double(GetTickCount() - start) / 1000.0;
#else
  time_t now;
  time(&now);
  return now - start_time_t;
#endif
}


static void print_uname() {
#ifdef HAVE_SYS_UTSNAME_H
  struct utsname u;
  if (uname(&u) == 0) {
    printf("%s %s %s %s %s\n",
           u.sysname, u.nodename, u.release, u.version, u.machine);
  }
#endif
}

// Generate stamp for this run
static void stamp_run(int iters) {
  time_t now = time(0);
  printf("======\n");
  fflush(stdout);
  print_uname();
  printf("Average over %d iterations\n", iters);
  fflush(stdout);
  // don't need asctime_r/gmtime_r: we're not threaded
  printf("Current time (GMT): %s", asctime(gmtime(&now)));
}

// If you have google-perftools (http://code.google.com/p/google-perftools),
// then you can figure out how much memory these implementations use
// as well.
#ifdef HAVE_GOOGLE_MALLOC_EXTENSION_H
#include <google/malloc_extension.h>

static size_t CurrentMemoryUsage() {
  size_t result;
  if (MallocExtension::instance()->GetNumericProperty(
          "generic.current_allocated_bytes",
          &result)) {
    return result;
  } else {
    return 0;
  }
}

#else  /* not HAVE_GOOGLE_MALLOC_EXTENSION_H */
static size_t CurrentMemoryUsage() { return 0; }

#endif

static void report(char const* title, double t,
                   int iters,
                   size_t start_memory, size_t end_memory) {
  // Construct heap growth report text if applicable
  char heap[100] = "";
  if (end_memory > start_memory) {
    snprintf(heap, sizeof(heap), "%7.1f MB",
             (end_memory - start_memory) / 1048576.0);
  }

  printf("%-20s %6.1f ns  (%8d hashes, %8d copies)%s\n",
         title, (t * 1000000000.0 / iters),
         NumHashesSinceLastCall(), NumCopiesSinceLastCall(),
         heap);
  fflush(stdout);
}

template<class MapType>
static void time_map_grow(int iters) {
  MapType set;
  Rusage t;

  SET_EMPTY_KEY(set, -2);
  const size_t start = CurrentMemoryUsage();
  t.Reset();
  for (int i = 0; i < iters; i++) {
    set[i] = i+1;
  }
  double ut = t.UserTime();
  const size_t finish = CurrentMemoryUsage();
  report("map_grow", ut, iters, start, finish);
}

template<class MapType>
static void time_map_grow_predicted(int iters) {
  MapType set;
  Rusage t;

  SET_EMPTY_KEY(set, -2);
  const size_t start = CurrentMemoryUsage();
  RESIZE(set, iters);
  t.Reset();
  for (int i = 0; i < iters; i++) {
    set[i] = i+1;
  }
  double ut = t.UserTime();
  const size_t finish = CurrentMemoryUsage();
  report("map_predict/grow", ut, iters, start, finish);
}

template<class MapType>
static void time_map_replace(int iters) {
  MapType set;
  Rusage t;
  int i;

  SET_EMPTY_KEY(set, -2);
  for (i = 0; i < iters; i++) {
    set[i] = i+1;
  }

  t.Reset();
  for (i = 0; i < iters; i++) {
    set[i] = i+1;
  }
  double ut = t.UserTime();

  report("map_replace", ut, iters, 0, 0);
}

template<class MapType>
static void time_map_fetch(int iters) {
  MapType set;
  Rusage t;
  int r;
  int i;

  SET_EMPTY_KEY(set, -2);
  for (i = 0; i < iters; i++) {
    set[i] = i+1;
  }

  r = 1;
  t.Reset();
  for (i = 0; i < iters; i++) {
    r ^= static_cast<int>(set.find(i) != set.end());
  }
  double ut = t.UserTime();

  srand(r);   // keep compiler from optimizing away r (we never call rand())
  report("map_fetch", ut, iters, 0, 0);
}

template<class MapType>
static void time_map_fetch_empty(int iters) {
  MapType set;
  Rusage t;
  int r;
  int i;

  SET_EMPTY_KEY(set, -2);
  r = 1;
  t.Reset();
  for (i = 0; i < iters; i++) {
    r ^= static_cast<int>(set.find(i) != set.end());
  }
  double ut = t.UserTime();

  srand(r);   // keep compiler from optimizing away r (we never call rand())
  report("map_fetch_empty", ut, iters, 0, 0);
}

template<class MapType>
static void time_map_remove(int iters) {
  MapType set;
  Rusage t;
  int i;

  SET_EMPTY_KEY(set, -2);
  for (i = 0; i < iters; i++) {
    set[i] = i+1;
  }

  t.Reset();
  SET_DELETED_KEY(set, -1);
  for (i = 0; i < iters; i++) {
    set.erase(i);
  }
  double ut = t.UserTime();

  report("map_remove", ut, iters, 0, 0);
}

template<class MapType>
static void time_map_toggle(int iters) {
  MapType set;
  Rusage t;
  int i;

  const size_t start = CurrentMemoryUsage();
  t.Reset();
  SET_DELETED_KEY(set, -1);
  SET_EMPTY_KEY(set, -2);
  for (i = 0; i < iters; i++) {
    set[i] = i+1;
    set.erase(i);
  }

  double ut = t.UserTime();
  const size_t finish = CurrentMemoryUsage();

  report("map_toggle", ut, iters, start, finish);
}

template<class MapType>
static void measure_map(const char* label, int obj_size, int iters) {
  printf("\n%s (%d byte objects, %d iterations):\n", label, obj_size, iters);
  if (1) time_map_grow<MapType>(iters);
  if (1) time_map_grow_predicted<MapType>(iters);
  if (1) time_map_replace<MapType>(iters);
  if (1) time_map_fetch<MapType>(iters);
  if (1) time_map_fetch_empty<MapType>(iters);
  if (1) time_map_remove<MapType>(iters);
  if (1) time_map_toggle<MapType>(iters);
}

template<class ObjType>
static void test_all_maps(int obj_size, int iters) {
  if (FLAGS_test_sparse_hash_map)
    measure_map< sparse_hash_map<ObjType, int, HashFn> >("SPARSE_HASH_MAP",
                                                 obj_size, iters);
  if (FLAGS_test_dense_hash_map)
    measure_map< dense_hash_map<ObjType, int, HashFn> >("DENSE_HASH_MAP",
                                                obj_size, iters);
#if defined(HAVE_UNORDERED_MAP)
  if (FLAGS_test_hash_map)
    measure_map< HASH_NAMESPACE::unordered_map<ObjType, int, HashFn> >(
        "TR1 UNORDERED_MAP", obj_size, iters);
#elif defined(HAVE_HASH_MAP)
  if (FLAGS_test_hash_map)
    measure_map< HASH_NAMESPACE::hash_map<ObjType, int, HashFn> >(
        "STANDARD HASH_MAP", obj_size, iters);
#endif
  if (FLAGS_test_map)
    measure_map< STL_NAMESPACE::map<ObjType, int, HashFn> >("STANDARD MAP",
                                     obj_size, iters);
}

int main(int argc, char** argv) {
  int iters = kDefaultIters;
  if (argc > 1) {            // first arg is # of iterations
    iters = atoi(argv[1]);
  }

  stamp_run(iters);

#ifndef HAVE_SYS_RESOURCE_H
  printf("\n*** WARNING ***: sys/resources.h was not found, so all times\n"
         "                 reported are wall-clock time, not user time\n");
#endif

  // It would be nice to set these at run-time, but by setting them at
  // compile-time, we allow optimizations that make it as fast to use
  // a HashObject as it would be to use just a straight int/char
  // buffer.  To keep memory use similar, we normalize the number of
  // iterations based on size.
  if (FLAGS_test_4_bytes)  test_all_maps< HashObject<4,4> >(4, iters/1);
  if (FLAGS_test_16_bytes)  test_all_maps< HashObject<16,16> >(16, iters/4);
  if (FLAGS_test_256_bytes)  test_all_maps< HashObject<256,256> >(256, iters/32);

  return 0;
}
