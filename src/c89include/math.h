/**/
#ifndef _MATH_H
#define _MATH_H

#ifdef __GNUC__  /* Needs at least GCC 3.3. */
#  define HUGE_VAL  (__builtin_huge_val())
#else
#  if defined(__TINYC__) || defined(__WATCOMC__)  /* Works in __GNUC__, __TINYC__ and __WATCOMC__, but it is a bit inefficient. */
    static __inline__ double __get_huge_val(void) { static union { unsigned long long __ll; double __d; } __u = { 0x7ff0000000000000ULL }; return __u.__d; }
#    define HUGE_VAL  (__get_huge_val())
#  else
#    error <math.h> does not know how to define HUGE_VAL.
#  endif
#endif

#endif  /* _MATH_H */
