/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "Vectors2d.h"

#include <math.h>
#include "SDL.h" // for M_PI

PolarVector2d PolarVector2dAdd(PolarVector2d v1, PolarVector2d v2)
{
  PolarVector2d v3;
  double aDif, mDurp;
  // convert angles to radians
  v1.Angle = v1.Angle * M_PI / 180;
  v2.Angle = v2.Angle * M_PI / 180;

  // from http://math.stackexchange.com/questions/1365622/adding-two-polar-vectors
  aDif = v2.Angle - v1.Angle;
  mDurp = v2.Magnitude * cos(aDif);
  v3.Magnitude = sqrt(v1.Magnitude * v1.Magnitude + v2.Magnitude * v2.Magnitude + 2 * v1.Magnitude * mDurp);
  v3.Angle = v1.Angle + atan2(v2.Magnitude * sin(aDif), v1.Magnitude + mDurp);

  // convert radians to angles
  v3.Angle = v3.Angle * 180 / M_PI;

  return v3;
}

CartesianVector2d PolarVector2dToCartesian(PolarVector2d v)
{
  CartesianVector2d v2;

  // convert angles to radians
  v.Angle = v.Angle * M_PI / 180;

  // convert polar to cartesian
  v2.X = v.Magnitude * cos(v.Angle);
  v2.Y = v.Magnitude * sin(v.Angle);

  return v2;
}

CartesianVector2d CartesianVector2dAdd(CartesianVector2d v1, CartesianVector2d v2)
{
  CartesianVector2d v3;
  v3.X = v1.X + v2.X;
  v3.Y = v1.Y + v2.Y;
  return v3;
}