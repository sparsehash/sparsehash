// Copyright (c) 2010, Google Inc.
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
// Author: Giao Nguyen

#include "config.h"
#include <google/sparsehash/sparsehashtable.h>
#include <google/sparsetable>   // for int16_t and u_int16_t
#include "testutil.h"
#include <stdio.h>
#include <stdlib.h>
#include <functional>   // for equal_to
#include <iostream>
#include <vector>
#include <string>

using STL_NAMESPACE::string;
using STL_NAMESPACE::vector;
using STL_NAMESPACE::cerr;
using STL_NAMESPACE::equal_to;
using STL_NAMESPACE::allocator;

static const int FLAGS_random_insertions = 1000000;
//             "Number of random insertions to test size and capacity"

#define arraysize(a)  ( sizeof(a) / sizeof(*(a)) )

namespace {

typedef int16_t int16;
typedef u_int16_t uint16;

using GOOGLE_NAMESPACE::sparse_hashtable;

template<typename Value>
struct Identity {
  Value& operator()(Value& v) const { return v; }
  const Value& operator()(const Value& v) const { return v; }
};

template<typename Value>
struct SetKey {
  void operator()(Value* value, const Value& new_key) const {
    *value = new_key;
  }
};

struct IntHash {
  size_t operator()(int16 v) const { return v; }
};

typedef sparse_hashtable<int16, int16,
                         IntHash, Identity<int16>, SetKey<int16>,
                         equal_to<int16>, allocator<int16> >  Table;

static int16 test_data[] = {
  -32767, -5432, -1, 0, 1, 8, 10000, 32767
};


class SparsehashtableTest {
 protected:
  virtual void SetUp() {
    for (int i = 0; i < arraysize(test_data); ++i) {
      table_.insert(test_data[i]);
    }
  }

  virtual void TearDown() {
  }

  Table table_;
};


// This test verifies basic functionalities of sparsehashtable
TEST_F(SparsehashtableTest, Basic) {
  // Test empty() on the preconstructed table
  EXPECT_FALSE(table_.empty());

  // Test default constructor and empty table
  Table ht;
  EXPECT_TRUE(ht.empty());
  EXPECT_EQ(ht.size(), 0);
  EXPECT_EQ(ht.bucket_count(), 32);  // min bucket count

  // Test operator==()
  EXPECT_TRUE(ht == ht);
  EXPECT_TRUE(ht != table_);
  EXPECT_TRUE(table_ == table_);

  // Test copy constructor
  Table ht2(table_);
  EXPECT_TRUE(ht2 == table_);

  // Test assignment
  ht = table_;
  EXPECT_TRUE(ht == table_);
  EXPECT_TRUE(ht == ht2);

  // Test clear()
  ht.clear();
  EXPECT_EQ(ht.size(), 0);
  EXPECT_FALSE(ht == ht2);
  EXPECT_TRUE(ht.empty());
  EXPECT_FALSE(ht2.empty());

  // Test swap()
  ht.swap(ht2);
  EXPECT_FALSE(ht.empty());
  EXPECT_TRUE(ht2.empty());
  EXPECT_FALSE(ht2 == table_);
  EXPECT_TRUE(ht == table_);
}


TEST_F(SparsehashtableTest, Insert) {
  // Test insert()
  Table ht;
  ht.insert(6);
  EXPECT_EQ(ht.size(), 1);
  ht.insert(8);
  EXPECT_EQ(ht.size(), 2);

  // Save a copy to compare later
  Table ht2(ht);

  // Inserting the same element won't change the size nor content
  ht.insert(6);
  EXPECT_EQ(ht.size(), 2);
  EXPECT_TRUE(ht == ht2);

  // Insert a different item will change the content
  ht.insert(9);
  EXPECT_EQ(ht.size(), 3);
  EXPECT_FALSE(ht == ht2);

  // Test insert(iterator, iterator); begin() and end()
  ht2.clear();
  EXPECT_EQ(ht2.size(), 0);
  ht2.insert(table_.begin(), table_.end());
  EXPECT_TRUE(ht2 == table_);
  EXPECT_FALSE(ht2 == ht);

  // Test resize() and inserting a lot of items
  ht.clear();
  Table::size_type num_buckets = ht.bucket_count();
  ht.resize(FLAGS_random_insertions);
  EXPECT_GT(ht.bucket_count(), num_buckets);
  cerr << "After reserving " << FLAGS_random_insertions
       << " buckets, size=" << ht.size()
       << " old_bucket_count=" << num_buckets
       << " new_bucket_count=" << ht.bucket_count() << "\n";
  num_buckets = ht.bucket_count();

  Table::size_type num_entries = ht.size();
  for (int i = 0; i < FLAGS_random_insertions; ++i) {
    int16 val = static_cast<int16>(rand() & 0xFFFF);  // signed -> unsigned
    num_entries += 1 - ht.count(val);
    ht.insert(val);
    EXPECT_EQ(ht.count(val), 1);
  }
  EXPECT_EQ(ht.size(), num_entries);
  // Check to see if the table got resized during insertion
  EXPECT_EQ(ht.bucket_count(), num_buckets);
  cerr << "After inserting " << FLAGS_random_insertions
       << " entries, size=" << ht.size()
       << " old_bucket_count=" << num_buckets
       << " new_bucket_count=" << ht.bucket_count() << "\n";
}


TEST_F(SparsehashtableTest, Find) {
  // Test find() and count()
  Table ht;
  for (int i = 0; i < arraysize(test_data); ++i) {
    int16 val = test_data[i];
    EXPECT_EQ(ht.count(val), 0);
    EXPECT_EQ(table_.count(val), 1);

    Table::const_iterator it = table_.find(val);
    EXPECT_TRUE(it != table_.end());
    EXPECT_EQ(*it, val);
  }
}


TEST_F(SparsehashtableTest, Erase) {
  Table ht(table_);
  EXPECT_TRUE(ht == table_);

  const int16 kDelKey = -32768;

  // Test set_deleted_key()
  ht.set_deleted_key(kDelKey);
  EXPECT_EQ(ht.deleted_key(), kDelKey);

  // Test erase(key)
  for (int i = 0; i < arraysize(test_data); ++i) {
    int16 val = test_data[i];
    EXPECT_EQ(ht.count(val), 1);
    ht.erase(val);
    EXPECT_EQ(ht.count(val), 0);
    EXPECT_EQ(ht.size(), table_.size() - i - 1);
  }
  EXPECT_EQ(ht.size(), 0);
  EXPECT_TRUE(ht.empty());

  // Test erase(iterator)
  ht = table_;
  ht.set_deleted_key(kDelKey);
  EXPECT_FALSE(ht.empty());
  EXPECT_TRUE(ht == table_);
  for (int i = 0; i < arraysize(test_data); ++i) {
    int16 val = test_data[i];
    Table::const_iterator it = ht.find(val);
    ht.erase(it);
    EXPECT_EQ(ht.count(val), 0);
    EXPECT_TRUE(ht.find(val) == ht.end());
  }

  // Test erase(iterator, iterator)
  ht = table_;
  ht.set_deleted_key(kDelKey);
  EXPECT_TRUE(ht == table_);
  ht.erase(table_.begin(), table_.end());
  EXPECT_FALSE(ht == table_);
  EXPECT_EQ(ht.size(), 0);
}


#ifndef _MSC_VER   // windows defines its own version
static string TmpFile(const char* basename) {
  return string("/tmp/") + basename;
}
#endif

TEST_F(SparsehashtableTest, ReadWrite) {
  // Test write_metadata(), read_metadata(), operator==()
  string test_path = TmpFile(".testfile.da");
  {
    FILE* fp = fopen(test_path.c_str(), "wb");
    if ( fp == NULL ) {
      // maybe we can't write to /tmp/.  Try the current directory
      test_path = ".testfile.da";
      fp = fopen(test_path.c_str(), "wb");
    }
    EXPECT_TRUE(fp);
    EXPECT_TRUE(table_.write_metadata(fp));
    EXPECT_TRUE(table_.write_nopointer_data(fp));
    fclose(fp);
  }

  FILE* fp = fopen(test_path.c_str(), "r");
  EXPECT_TRUE(fp);
  Table ht;
  EXPECT_TRUE(ht.read_metadata(fp));
  EXPECT_TRUE(ht.read_nopointer_data(fp));
  EXPECT_TRUE(ht == table_);
}

}  // namespace


int main(int argc, char **argv) {
  // Tests are run via global constructors, so we have nothing to do.
  printf("\nSizeOf: sparse_hashtable<int16> = %d\n",
         static_cast<int>(sizeof(Table)));
  printf("PASS.\n");
  return 0;
}
