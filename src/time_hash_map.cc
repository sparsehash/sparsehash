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
// See PERFORMANCE for the output of one example run.

#include <google/sparsehash/config.h>
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
}

// The functions that we call on each map, that differ for different types.
// By default each is a noop, but we redefine them for types that need them.

#include <map>
#include "hash_map.h"
#include <google/sparse_hash_map>
#include <google/dense_hash_map>

using GOOGLE_NAMESPACE::sparse_hash_map;
using GOOGLE_NAMESPACE::dense_hash_map;
using HASH_NAMESPACE::hash_map;
using STL_NAMESPACE::map;

// Normally I don't like non-const references, but using them here ensures
// the inlined code ends up as efficient as possible.

template<class MapType> inline void SET_DELETED_KEY(MapType& map, int key) {}
template<class MapType> inline void SET_EMPTY_KEY(MapType& map, int key) {}
template<class MapType> inline void RESIZE(MapType& map, int iters) {}

template<> inline void SET_DELETED_KEY(sparse_hash_map<int, int>& m, int key) {
  m.set_deleted_key(key);
}
template<> inline void SET_DELETED_KEY(dense_hash_map<int, int>& m, int key) {
  m.set_deleted_key(key);
}

template<> inline void SET_EMPTY_KEY(dense_hash_map<int, int>& m, int key) {
  m.set_empty_key(key);
}

template<> inline void RESIZE(sparse_hash_map<int, int>& m, int iters) {
  m.resize(iters);
}
template<> inline void RESIZE(dense_hash_map<int, int>& m, int iters) {
  m.resize(iters);
}
template<> inline void RESIZE(hash_map<int, int>& m, int iters) {
  m.resize(iters);
}

static const int default_iters = 10000000;


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
#ifdef HAVE_SYS_RESOURCE_H
  struct rusage start;
#else
  time_t start_time_t;
#endif
};

inline void Rusage::Reset() {
#ifdef HAVE_SYS_RESOURCE_H
  getrusage(RUSAGE_SELF, &start);
#else
  time(&start_time_t);
#endif
}

inline double Rusage::UserTime() {
#ifdef HAVE_SYS_RESOURCE_H
  struct rusage u;
  
  getrusage(RUSAGE_SELF, &u);
  
  struct timeval result;
  result.tv_sec  = u.ru_utime.tv_sec  - start.ru_utime.tv_sec;
  result.tv_usec = u.ru_utime.tv_usec - start.ru_utime.tv_usec;
  
  return double(result.tv_sec) + double(result.tv_usec) / 1000000.0;
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
                   size_t heap_growth) {
  // Construct heap growth report text if applicable
  char heap[100];
  if (heap_growth > 0) {
    snprintf(heap, sizeof(heap), "%8.1f MB", heap_growth / 1048576.0);
  } else {
    heap[0] = '\0';
  }
  
  printf("%-20s %8.1f ns %s\n", title, (t * 1000000000.0 / iters), heap);
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
  report("map_grow", ut, iters, finish - start);
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
  report("map_predict/grow", ut, iters, finish - start);
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

  report("map_replace", ut, iters, 0);
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
    r ^= (set.find(i) != set.end());
  }
  double ut = t.UserTime();

  report("map_fetch", ut, iters, 0);
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
    r ^= (set.find(i) != set.end());
  }
  double ut = t.UserTime();

  report("map_fetch_empty", ut, iters, 0);
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

  report("map_remove", ut, iters, 0);
}

template<class MapType>
static void measure_map(const char* label, int iters) {
  printf("\n%s:\n", label);
  if (1) time_map_grow<MapType>(iters);
  if (1) time_map_grow_predicted<MapType>(iters);
  if (1) time_map_replace<MapType>(iters);
  if (1) time_map_fetch<MapType>(iters);
  if (1) time_map_fetch_empty<MapType>(iters);
  if (1) time_map_remove<MapType>(iters);
}

int main(int argc, char** argv) {
  int iters = default_iters;
  if (argc > 1) {            // first arg is # of iterations
    iters = atoi(argv[1]);
  }

  stamp_run(iters);

#ifndef HAVE_SYS_RESOURCE_H
  printf("\n*** WARNING ***: sys/resources.h was not found, so all times\n"
         "                 reported are wall-clock time, not user time\n");
#endif

  if (1) measure_map< sparse_hash_map<int, int> >("SPARSE_HASH_MAP", iters);
  if (1) measure_map< dense_hash_map<int, int> >("DENSE_HASH_MAP", iters);
  if (1) measure_map< hash_map<int, int> >("STANDARD HASH_MAP", iters);
  if (1) measure_map< map<int, int> >("STANDARD MAP", iters);

  return 0;
}
