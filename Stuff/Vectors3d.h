/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#ifndef NATE_COMMANDER_VECTORS_3D_H
#define NATE_COMMANDER_VECTORS_3D_H

typedef struct PolarVector3d
{
  double XYAngle;
  double YZAngle;
  double Magnitude;
} PolarVector3d;

typedef struct CartesianVector3d
{
  double X;
  double Y;
  double Z;
} CartesianVector3d;

PolarVector3d CartesianVector3dToPolar(CartesianVector3d v);

#endif