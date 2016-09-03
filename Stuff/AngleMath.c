/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "AngleMath.h"

#include <math.h>

double AngleMath_GetAbsAnglesBetween(double a1, double a2)
{
  double best, next, modified_a1;

  // TODO: do some modulo to guarantee this doesn't take WAY too long for huge values

  // initial best
  best = fabs(a2 - a1);
  
  // add 360 to a1 until it's as close to a2 as possible
  modified_a1 = a1 + 360;
  next = fabs(a2 - modified_a1);
  while (next < best)
  {
    best = next;
    modified_a1 += 360;
    next = fabs(a2 - modified_a1);
  }

  // subtract 360 from a1 until it's as close to a2 as possible
  modified_a1 = a1 - 360;
  next = fabs(a2 - modified_a1);
  while (next < best)
  {
    best = next;
    modified_a1 -= 360;
    next = fabs(a2 - modified_a1);
  }

  // return the best found number
  return best;
}