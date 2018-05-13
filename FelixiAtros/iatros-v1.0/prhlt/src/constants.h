// Define some constants not defined by the standard C99
// although they are defined in GNU99
#include <prhlt/config.h>

#ifdef HAVE_VALUES_H
  #include <values.h>
#endif

#ifndef INFINITY
  #ifdef __GNUC__
    #define INFINITY (__builtin_inff())
  #else
    #define INFINITY FLT_MAX
  #endif
#endif

#include <float.h>
// if M_E is not defined set math constants
#ifndef M_E
// from math.h:
#define M_E            2.7182818284590452354   /* e */
#define M_LOG2E        1.4426950408889634074   /* log_2 e */
#define M_LOG10E       0.43429448190325182765  /* log_10 e */
#define M_LN2          0.69314718055994530942  /* log_e 2 */
#define M_LN10         2.30258509299404568402  /* log_e 10 */
#define M_PI           3.14159265358979323846  /* pi */
#define M_PI_2         1.57079632679489661923  /* pi/2 */
#define M_PI_4         0.78539816339744830962  /* pi/4 */
#define M_1_PI         0.31830988618379067154  /* 1/pi */
#define M_2_PI         0.63661977236758134308  /* 2/pi */
#define M_2_SQRTPI     1.12837916709551257390  /* 2/sqrt(pi) */
#define M_SQRT2        1.41421356237309504880  /* sqrt(2) */
#define M_SQRT1_2      0.70710678118654752440  /* 1/sqrt(2) */
#endif // math constants

#define MAX_LINE 1024

#if !defined(LOG_ZERO)
#define LOG_ZERO -FLT_MAX
#endif
#define is_logzero(x) (x <= LOG_ZERO)