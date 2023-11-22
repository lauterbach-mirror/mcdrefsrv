/** \file mcd_types.h
    \brief The Multi-core Debug (MCD) API basic types

    This is the definition of basic scalar types used by the SPRINT API.
    
    New host platforms may require changes in this file.
*/


#ifndef __mcd_types_h
#define __mcd_types_h

#ifdef _MSC_VER
typedef __int8            int8_t;
typedef __int16           int16_t;
typedef __int32           int32_t;
typedef __int64           int64_t;
typedef unsigned __int8   uint8_t;
typedef unsigned __int16  uint16_t;
typedef unsigned __int32  uint32_t;
typedef unsigned __int64  uint64_t;
#else
#if defined(__SUNPRO_C) || defined(__hpux)
#include <inttypes.h>
#else
#include <stdint.h>
#endif
#endif

/* Replacement for stdbool.h */
#ifndef __cplusplus

#if defined(__GNUC__) && __GNUC__ >= 3

#undef	bool
#undef	true
#undef	false

/* Support for _C99: type _Bool is already built-in. */
#define false	0
#define true	1

/* User visible type `bool' is provided as a macro which may be redefined */
#define bool _Bool

#else

/* `bool' type must promote to `int' or `unsigned int'. */
typedef enum {
	false = 0,
	true = 1
} bool;

#endif

#endif /* __cplusplus */

/* Inform that everything is fine */
#define __bool_true_false_are_defined 1

#endif


