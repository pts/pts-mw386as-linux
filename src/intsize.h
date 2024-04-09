#ifndef _INTSIZE_H_
#define _INTSIZE_H_

#include <stddef.h>

#if defined(PRId32) || (defined(__STDC__) && __STDC_VERSION__ >= 199901L)
#  if defined(int32_t)  /* Respect `cc -Dint32_t=long -DPRId32=ld -DPRIx32=lx' override from the command-line. */
#    ifndef uint32_t
#      define uint32_t unsigned int32_t
#    endif
#  else  /* C99 and C++11. */
#    include <stdint.h>
#    include <inttypes.h>  /* PRId32. */
    typedef int32_t i32_t;
    typedef uint32_t u32_t;
#  endif
#else
  /* This is not C89, but there is no better way. */
#  if __SIZEOF_INT__ == 4 || __INTSIZE == 4 || __WORDSIZE == 32 || (defined(_M_X64) || defined(_M_AMD64) || defined(__x86_64__) || defined(__amd64__) || defined(__i386__) || defined(__i386) || (defined(_M_IX86) && !defined(_M_I86)) || defined(__386) || defined(__386__) || defined(__X86_64__) || defined(_M_I386) || defined(_M_ARM) || defined(_M_ARM64) || defined(__m68k__) || defined(__ia64__) || defined(_M_IA64) || defined(__powerpc__) || defined(_M_PPC))
    typedef int i32_t;  /* Avoid using `long', that's 8 bytes with GCC on amd64. */
    typedef unsigned u32_t;
#    define PRId32 "d"
#    define PRIx32 "x"
#    define PRIX32 "X"
#  else
    typedef long i32_t;
    typedef unsigned long u32_t;
#    define PRId32 "ld"
#    define PRIx32 "lx"
#    define PRIX32 "lX"
#  endif
#endif

typedef char assert_i32_t_size[sizeof(i32_t) == 4 ? 1 : -1];
typedef char assert_int_size[sizeof(int) == 4 ? 1 : -1];  /* This is extra precaution. TODO(pts): Verify that it works with 2 as well. */

#endif  /* _INTSIZE_H_ */
