/**/
#ifndef _TIME_H
#define _TIME_H

#include <stddef.h>  /* size_t. */

#if !defined(__i386__) && !defined(__amd64__)
#  error <time.h> is written only for i386.  /* Because of time_t. */
#endif

typedef long time_t;  /* glibc has int32_t for i386 and int64_t for amd64. */
time_t time(time_t *tloc);
char *ctime(const time_t *timep);

#endif  /* _TIME_H */
