#include "ViewGraphics.h"

#include <Windows.h>
#include <GL/gl.h>

// view angle (x=0 y=0) looks down the Z axis from + to -
// view angle (x+) rotates camera right (initially shows more x+)
// view angle (y+) rotates camera up (initially shows more y+)
// x axis: -1 is left, +1 is right
// y axis: -1 is down, +1 is up
// z axis: -1 is into the monitor, +1 is toward me
void SetView_CameraAtPoint_LookingAtAngle(float * xyzCameraPosition, float leftRightAngle, float upDownAngle)
{
  // first (TODO: why first? why must rotation be done before translation?)
  // rotate all the world objects in the negative-as-desired direction 
  // to simulate camera angle.

  // left-right is implemented as "around the y axis"
  // up-down is implemented as "around the x axis"
  glRotated(-upDownAngle, 1.0f, 0.0f, 0.0f);
  glRotated(leftRightAngle, 0.0f, 1.0f, 0.0f);

  // next (TODO: why next? why can we translate in world units AFTER rotating?)
  // translate all the world objects in the negative-as-desired direction
  // to simulate the camera position
  glTranslatef(-xyzCameraPosition[0], -xyzCameraPosition[1], -xyzCameraPosition[2]);
}

void SetView_CameraLookingAtPoint_FromDistance_AtAngle(
  float * xyzFocalPoint, float distanceFromFocalPoint, float leftRightAngle, float upDownAngle)
{
  // VERY FIRST do the distance thing
  // TODO: why? 
  glTranslatef(0.0f, 0.0f, -distanceFromFocalPoint);
  
  // then rotate and translate to the desired point
  SetView_CameraAtPoint_LookingAtAngle(xyzFocalPoint, leftRightAngle, upDownAngle);
}

