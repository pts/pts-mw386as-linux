/**/
#ifndef _STDDEF_H
#define _STDDEF_H

typedef unsigned long size_t;
typedef char assert_size_t_size[sizeof(size_t) == sizeof(void*) ? 1 : -1];

#endif  /* _STDDEF_H */