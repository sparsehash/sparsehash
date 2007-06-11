/* Handmade config.h for windows.  This is intended to be used with VC++,
   which does not use the unix-based './configure' configuration tool.
   These settings were taken from this patch, by Tobias Polzin:
      http://code.google.com/p/google-sparsehash/issues/attachment?aid=-71112522921222597&name=patch-sparsehash-0.6-win

   Note: when updating this file, you should include values for all the
         variables in src/config.h.include.
*/

/***
 *** Feel free to change these values to whatever suits your fancy
 ***/
#define GOOGLE_NAMESPACE  google

/***
 *** These are windows-specific; not from config.h.include
 ***/
#define HAVE_WINDOWS_H  1
#define snprintf  _snprintf
#define _DEFINE_DEPRECATED_HASH_CLASSES  0

/***
 *** Do not change these values unless you know they are wrong for your system
 ***/
#define HASH_FUN_H <google/sparsehash/stl_hash_fun.h>
#define HASH_MAP_H <hash_map>
#define HASH_NAMESPACE  stdext
/* #undef HAVE_UINT16_T */
/* #undef HAVE_U_INT16_T */
#define HAVE___UINT16  1
#define HAVE_LONG_LONG  1
/* #undef HAVE_SYS_TYPES_H */
/* #undef HAVE_STDINT_H */
/* #undef HAVE_INTTYPES_H */
#define HAVE_MEMCPY  1
#define STL_NAMESPACE  std
#define _END_GOOGLE_NAMESPACE_    };
#define _START_GOOGLE_NAMESPACE_  namespace GOOGLE_NAMESPACE {

/***
 *** These are only needed for the unittests and could go outside config.h
 ***/
/* #undef HAVE_SYS_RESOURCE_H */
/* #undef HAVE_UNISTD_H */
/* #undef HAVE_SYS_TIME_H */
/* #undef HAVE_SYS_UTSNAME_H */
#define HAVE_HASH_MAP  1
/* #undef HAVE_GOOGLE_MALLOC_EXTENSION_H */
