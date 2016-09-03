/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "Vectors3d.h"

#include <math.h>
#include "SDL.h" // for M_PI
#include "SafeMath.h"

PolarVector3d CartesianVector3dToPolar(CartesianVector3d v)
{
  PolarVector3d result;
  result.Magnitude = sqrt(v.X * v.X + v.Y * v.Y + v.Z * v.Z);
  result.XYAngle = SafeMath_CoolAtan2(v.Y, v.X) * 180 / M_PI;
  result.YZAngle = SafeMath_CoolAtan2(v.Z, sqrt(v.X * v.X + v.Y * v.Y)) * 180 / M_PI;
  return result;
}
