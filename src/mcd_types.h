/** \file mcd_types.h
	\brief The Multi-core Debug (MCD) API basic types

	This is the definition of basic scalar types used by the Lauterbach version of the SPRINT API.
	In contrast to the SPRINT MCD API version 1.0 boolean types and char types are here defined with
	a size, which is fixed for the same platform.

	New host platforms may require changes in this file.
*/

#ifndef __mcd_types_h
#define __mcd_types_h


#if defined(_MSC_VER) && (_MSC_VER<1600)
  typedef __int8            int8_t;
  typedef __int16           int16_t;
  typedef __int32           int32_t;
  typedef __int64           int64_t;
  typedef unsigned __int8   uint8_t;
  typedef unsigned __int16  uint16_t;
  typedef unsigned __int32  uint32_t;
  typedef unsigned __int64  uint64_t;
#else
# if defined(__SUNPRO_C) || defined(__SUNPRO_CC) || defined(__hpux)
#   include <inttypes.h>
# else
#   include <stdint.h>
# endif
#endif


/* Definition of character type (mcd_char_t):
 * Characters are only used for strings.
 * We assume here that "char" has the same size for all compilers on a platform.
 * The width of a character should correspond to the size of a minimum addressable unit.
 * So for byte addressable machines the mcd_char_t should have the width of 8-bits.
 */
typedef char mcd_char_t;


/* Definition of the boolean type (mcd_bool_t):
 * The boolean types must have always the same size with any compiler on the same platform.
 * Booleans are here defined with 8-bits or 32-bits depending on the used platform to have optimal compatibility
 * with the original SPRINT MCD API 1.0.
 * -> On Linux and MacOS X operating systems booleans must have 8-bits
 * -> On all other platforms booleans must have 32-bits (stored in target Endianess)
 * Note, that both C and C++ standard define their boolean type (bool or _Bool) not with a fixed size.
 */
#if defined(__linux__) || defined(__APPLE__)
typedef uint8_t  mcd_bool_t;
#else
typedef uint32_t mcd_bool_t;
#endif

#if !defined(FALSE)
# define FALSE 0
#endif
#if !defined(TRUE)
# define TRUE  1
#endif


#endif /* __mcd_types_h */

