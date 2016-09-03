/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "SafeMath.h"

#include <math.h>
#include "SDL.h" // for M_PI

#include <float.h> // for floating point error checking on windows

#define SAFEMATH_PERFORM(x) \
  unsigned int rv; \
  \
  _clearfp(); \
  *result = (x); \
  rv = _clearfp(); \
  \
  if (0 == (rv & (_SW_UNDERFLOW | _SW_OVERFLOW | _SW_ZERODIVIDE | _SW_INVALID)) && \
      0 != (_fpclass(*result) & (_FPCLASS_NN | _FPCLASS_ND | _FPCLASS_NZ | _FPCLASS_PZ | _FPCLASS_PD | _FPCLASS_PN))) \
  { \
    return 1; \
  } \
  else \
  { \
    return 0; \
  }

// returns 1 if the divide executed flawlessly
int SafeMath_Divide(double numerator, double denominator, double * result)
{
  SAFEMATH_PERFORM(numerator / denominator);
}

int SafeMath_Multiply(double a, double b, double * result)
{
  SAFEMATH_PERFORM(a * b);
}

int SafeMath_Add(double a, double b, double * result)
{
  SAFEMATH_PERFORM(a + b);
}

int SafeMath_Sqrt(double a, double * result)
{
  SAFEMATH_PERFORM(sqrt(a)); // TODO: not sure if fp is meaningful after calling this
}

int SafeMath_Atan(double a, double * result)
{
  SAFEMATH_PERFORM(atan(a)); // TODO: not sure if fp is meaningful after calling this
}

int SafeMath_Acos(double a, double * result)
{
  SAFEMATH_PERFORM(acos(a)); // TODO: not sure if fp is meaningful after calling this
}

int SafeMath_Asin(double a, double * result)
{
  SAFEMATH_PERFORM(asin(a)); // TODO: not sure if fp is meaningful after calling this
}

double SafeMath_CoolAtan2(double y, double x)
{
  int yClass, xClass, divClass;
  double yDivX;

  // normal atan2 does this (says MSDN)
  // if x equals 0, atan2 returns p/2 if y is positive, -p/2 if y is negative, or 0 if y is 0.

  // cool atan2 returns more varied angles for infinities, and NAN for NAN scenarios

  yClass = _fpclass(y);
  xClass = _fpclass(x);

  if (yClass & (_FPCLASS_SNAN | _FPCLASS_QNAN))
  {
    return yClass;
  }

  if (xClass & (_FPCLASS_SNAN | _FPCLASS_QNAN))
  {
    return xClass;
  }

  if (yClass & (_FPCLASS_NINF))
  {
    if (xClass & (_FPCLASS_NINF))
    {
      return M_PI * 5 / 4;
    }
    else if (xClass & (_FPCLASS_PINF))
    {
      return M_PI * 7 / 4;
    }
    else
    {
      return M_PI * 3 / 2;
    }
  }

  if (yClass & (_FPCLASS_PINF))
  {
    if (xClass & (_FPCLASS_NINF))
    {
      return M_PI * 3 / 4;
    }
    else if (xClass & (_FPCLASS_PINF))
    {
      return M_PI * 1 / 4;
    }
    else
    {
      return M_PI / 2;
    }
  }

  if (xClass & (_FPCLASS_NINF))
  {
    return M_PI;
  }

  if (xClass & (_FPCLASS_PINF))
  {
    return 0;
  }

  // Floating point division by zero has well-defined results: 
  //   either positive or negative infinity if the numerator is nonzero, 
  //   or NAN if the numerator is zero.
  // It's also possible to get infinity when the denominator is nonzero 
  // but very close to zero (e.g. subnormal).
  yDivX = y / x;
  divClass = _fpclass(yDivX);

  if (divClass & (_FPCLASS_NINF | _FPCLASS_PINF))
  {
    if (y > 0)
    {
      return M_PI / 2;
    }
    else
    {
      return M_PI * 3 / 2;
    }
  }

  if (divClass & (_FPCLASS_SNAN | _FPCLASS_QNAN))
  {
    return 0;
  }

  return atan(yDivX);
}