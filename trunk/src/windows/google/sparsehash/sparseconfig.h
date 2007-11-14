#ifndef SPARSEHASH_WINDOWS_SPARSECONFIG_H__
#define SPARSEHASH_WINDOWS_SPARSECONFIG_H__

/***
 *** These are #defines that autoheader puts in config.h.in that we
 *** want to show up in sparseconfig.h, the minimal config.h file
 *** #included by all our .h files.  The reason we don't take
 *** everything that autoheader emits is that we have to include a
 *** config.h in installed header files, and we want to minimize the
 *** number of #defines we make so as to not pollute the namespace.
 ***/
#define GOOGLE_NAMESPACE  google
#define HASH_NAMESPACE  stdext
#define HASH_FUN_H   <hash_map>
#define SPARSEHASH_HASH  HASH_NAMESPACE::hash_compare
#undef HAVE_UINT16_T
#undef HAVE_U_INT16_T
#define HAVE___UINT16  1
#define HAVE_LONG_LONG  1
#define HAVE_SYS_TYPES_H  1
#undef HAVE_STDINT_H
#undef HAVE_INTTYPES_H
#define HAVE_MEMCPY  1
#define STL_NAMESPACE  std
#define _END_GOOGLE_NAMESPACE_  }
#define _START_GOOGLE_NAMESPACE_  namespace GOOGLE_NAMESPACE {


// ---------------------------------------------------------------------
// Extra stuff not found in config.h.include

#define WIN32_LEAN_AND_MEAN  /* We always want minimal includes */
#include <windows.h>         /* TODO(csilvers): do in every .h file instead? */

#endif  /* SPARSEHASH_WINDOWS_SPARSECONFIG_H__ */
