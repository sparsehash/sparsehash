## Process this file with autoconf to produce configure.
## In general, the safest way to proceed is to run the following:
##
##    % aclocal -I `pwd`/../autoconf && autoheader && fgrep -B2 -f src/config.h.include src/google/sparsehash/config.h.in | fgrep -vx -e -- > _sparsehash_config && mv -f _sparsehash_config src/google/sparsehash/config.h.in && autoconf && automake
##
## NOTE THE GREP!  We do a little non-standard thing for this package.

# make sure we're interpreted by some minimal autoconf
AC_PREREQ(2.57)

AC_INIT(sparsehash, 0.2, opensource@google.com)
# The argument here is just something that should be in the current directory
# (for sanity checking)
AC_CONFIG_SRCDIR(README)
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(src/google/sparsehash/config.h)

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX

# Check whether some low-level functions/files are available
AC_HEADER_STDC
AC_CHECK_FUNCS(memcpy memmove)
AC_CHECK_TYPES([u_int16_t])    # defined in most posix systems
AC_CHECK_TYPES([__uint16])     # defined in some windows systems (vc7)
# These are 'only' needed for unittests
AC_CHECK_HEADERS(sys/resource.h unistd.h sys/time.h sys/utsname.h)

# Writes the header files for hash_map, hash_set, and hash_fun into .
# This also tells us what namespace hash code lives in.
AC_CXX_MAKE_HASH_MAP_H(src/hash_map.h)
AC_CXX_MAKE_HASH_FUN_H(src/google/sparsehash/hash_fun.h)

# Find out what namespace 'normal' STL code lives in, and also what namespace
# the user wants our classes to be defined in
AC_CXX_STL_NAMESPACE
AC_DEFINE_GOOGLE_NAMESPACE(google)

# Check what STL features are defined
AC_CXX_STL_ITERATOR_TAGS

# TODO(csilvers): turn on again once we decide whether we want to enforce
#                 DECLARE_POD with the hash types
#AC_CXX_STL_TYPE_TRAITS

# Write generated configuration file
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
