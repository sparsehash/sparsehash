## Process this file with automake to produce Makefile.in

# Make sure that when we re-make ./configure, we get the macros we need
ACLOCAL_AMFLAGS = -I `pwd`/../autoconf

# This is so we can #include <google/foo>
AM_CPPFLAGS = -I$(top_srcdir)/src

googleincludedir = $(includedir)/google
## The .h files you want to install (that is, .h files that people
## who install this package can include in their own applications.)
googleinclude_HEADERS =				\
   src/google/dense_hash_map			\
   src/google/dense_hash_set			\
   src/google/sparse_hash_map			\
   src/google/sparse_hash_set			\
   src/google/sparsetable                       \
   src/google/type_traits.h

docdir = $(prefix)/share/doc/$(PACKAGE)-$(VERSION)
## This is for HTML and other documentation you want to install.
## Add your documentation files (in doc/) in addition to these boilerplate
## Also add a TODO file if you have one
dist_doc_DATA = AUTHORS COPYING ChangeLog INSTALL NEWS README TODO \
   doc/dense_hash_map.html			\
   doc/dense_hash_set.html			\
   doc/sparse_hash_map.html			\
   doc/sparse_hash_set.html			\
   doc/sparsetable.html                     	\
   doc/implementation.html			\
   doc/performance.html                         \
   doc/index.html                               \
   doc/designstyle.css

## The libraries (.so's) you want to install
lib_LTLIBRARIES =

## unittests you want to run when people type 'make check'.
## TESTS is for binary unittests, check_SCRIPTS for script-based unittests.
## TESTS_ENVIRONMENT sets environment variables for when you run unittest,
## but it only seems to take effect for *binary* unittests (argh!)
TESTS = type_traits_unittest sparsetable_unittest hashtable_unittest
check_SCRIPTS =
TESTS_ENVIRONMENT =

## This should always include $(TESTS), but may also include other
## binaries that you compile but don't want automatically installed.
noinst_PROGRAMS = $(TESTS) time_hash_map


## vvvv RULES TO MAKE THE LIBRARIES, BINARIES, AND UNITTESTS

# This doesn't include the config.h file, but everything else we make
configure_generated_h_files =			\
   src/google/sparsehash/hash_fun.h		\
   src/hash_map.h

sparsehashincludedir = $(googleincludedir)/sparsehash
sparsehashinclude_HEADERS =			\
   src/google/sparsehash/hash_fun.h		\
   src/google/sparsehash/densehashtable.h	\
   src/google/sparsehash/sparsehashtable.h	\
   src/google/sparsehash/config.h

type_traits_unittest_SOURCES =			\
   src/type_traits_unittest.cc			\
   src/google/sparsehash/config.h		\
   src/google/type_traits.h

sparsetable_unittest_SOURCES =			\
   src/sparsetable_unittest.cc			\
   src/google/sparsehash/config.h		\
   src/google/sparsetable			\
   src/sparsetable_unittest.expected

hashtable_unittest_SOURCES =			\
   src/hashtable_unittest.cc			\
   $(googleinclude_HEADERS)			\
   $(sparsehashinclude_HEADERS)			\
   src/words

time_hash_map_SOURCES =				\
   src/time_hash_map.cc				\
   src/google/sparsehash/config.h		\
   src/hash_map.h				\
   $(googleinclude_HEADERS)

# If tcmalloc is installed, use it with time_hash_map; it gives us
# heap-usage statistics for the hash_map routines, which is very nice
time_hash_map_LDADD = @tcmalloc@

## ^^^^ END OF RULES TO MAKE THE LIBRARIES, BINARIES, AND UNITTESTS


rpm: dist-gzip packages/rpm.sh packages/rpm/rpm.spec
	@cd packages && ./rpm.sh ${PACKAGE} ${VERSION}

deb: dist-gzip packages/deb.sh packages/deb/*
	@cd packages && ./deb.sh ${PACKAGE} ${VERSION}

EXTRA_DIST = packages/rpm.sh packages/rpm/rpm.spec packages/deb.sh packages/deb \
             src/config.h.include src/windows experimental

DISTCLEANFILES = $(configure_generated_h_files)
