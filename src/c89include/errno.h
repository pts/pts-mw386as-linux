/**/
#ifdef USE_SIMPLE_ERRNO
  extern int errno;
#else
  /* This is glibc and uClibc. With just `extern int errno', linking would fail with some .tbss error. */
  extern int *__errno_location(void);
#  define errno (*__errno_location ())
#endif

/* This is the Linux value, but the actual value may not matter for the program. */
#define ERANGE          34      /* Math result not representable */
