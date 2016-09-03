/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#ifndef NATE_COMMANDER_SAFE_MATH_H
#define NATE_COMMANDER_SAFE_MATH_H

// returns 1 if the operation executed flawlessly
int SafeMath_Divide(double numerator, double denominator, double * result);
int SafeMath_Multiply(double a, double b, double * result);
int SafeMath_Add(double a, double b, double * result);
int SafeMath_Sqrt(double a, double * result);
int SafeMath_Atan(double a, double * result);
int SafeMath_Acos(double a, double * result);
int SafeMath_Asin(double a, double * result);
double SafeMath_CoolAtan2(double y, double x);

#endif