/**/
#ifndef _TIME_H
#define _TIME_H

#include <stddef.h>  /* size_t. */

#ifndef __i386__
#  error <time.h> is written only for i386.
#endif
typedef long time_t;
time_t time(time_t *tloc);
char *ctime(const time_t *timep);

#endif  /* _TIME_H */
