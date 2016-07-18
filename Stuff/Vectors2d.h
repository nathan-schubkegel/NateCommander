#ifndef NATE_COMMANDER_VECTORS_2D_H
#define NATE_COMMANDER_VECTORS_2D_H

typedef struct PolarVector2d
{
  double Angle;
  double Magnitude;
} PolarVector2d;

typedef struct CartesianVector2d
{
  double X;
  double Y;
} CartesianVector2d;

PolarVector2d PolarVector2dAdd(PolarVector2d v1, PolarVector2d v2);
CartesianVector2d PolarVector2dToCartesian(PolarVector2d v);
CartesianVector2d CartesianVector2dAdd(CartesianVector2d v1, CartesianVector2d v2);

#endif