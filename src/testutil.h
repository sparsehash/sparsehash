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
// Author: Craig Silverstein

// This macro mimics a unittest framework, but is a bit less flexible
// than most.  It requires a superclass to derive from, and does all
// work in global constructors.

#include <stdio.h>

#define TEST_F(superclass, testname)                    \
  class TEST_##testname : public superclass {           \
   public:                                              \
    TEST_##testname() {                                 \
      fputs("Running " #testname "\n", stderr);         \
      SetUp();                                          \
      Run();                                            \
      TearDown();                                       \
    }                                                   \
    void Run();                                         \
  };                                                    \
  static TEST_##testname test_instance_##testname;      \
  void TEST_##testname::Run()

#define EXPECT_TRUE(cond)  do {                 \
  if (!(cond)) {                                \
    fputs("Test failed: " #cond "\n", stderr);  \
    exit(1);                                    \
  }                                             \
} while (0)

#define EXPECT_FALSE(a)  EXPECT_TRUE(!(a))
#define EXPECT_EQ(a, b)  EXPECT_TRUE((a) == (b))
#define EXPECT_LT(a, b)  EXPECT_TRUE((a) < (b))
#define EXPECT_GT(a, b)  EXPECT_TRUE((a) > (b))
