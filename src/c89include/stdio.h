/**/
#ifndef _STDIO_H
#define _STDIO_H

#include <stdarg.h>  /* For vfprintf(...). TODO(pts): Define internally only. */
#include <stddef.h>  /* size_t. */

#define NULL ((void*)0)
typedef struct _FILE *FILE;
extern FILE *stdin, *stdout, *stderr;
#define EOF -1
int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int sprintf(char *str, const char *format, ...);
int vfprintf(FILE *stream, const char *format, va_list ap);
#if 0  /* Unused. */
  void perror(const char *s);
#endif
FILE *fopen(const char *pathname, const char *mode);
int fputc(int c, FILE *stream);
int fputs(const char *s, FILE *stream);
int fgetc(FILE *stream);
char *fgets(char *s, int size, FILE *stream);
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
int fclose(FILE *stream);
int sscanf(const char *str, const char *format, ...);
int remove(const char *pathname);

#endif  /* _STDIO_H */
