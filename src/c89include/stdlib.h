/**/
#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h>  /* size_t. */

#define NULL ((void*)0)
long strtol(const char *nptr, char **endptr, int base);
void exit(int exit_code);
void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
char *getenv(const char *name);
void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));

#endif  /* _STDLIB_H */
