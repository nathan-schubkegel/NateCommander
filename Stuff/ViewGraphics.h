#ifndef NATE_COMMANDER_VIEW_GRAPHICS_H
#define NATE_COMMANDER_VIEW_GRAPHICS_H

// view angle (x=0 y=0) looks down the Z axis from + to -
// view angle (x+) rotates camera right (initially shows more x+)
// view angle (y+) rotates camera up (initially shows more y+)
// x axis: -1 is left, +1 is right
// y axis: -1 is down, +1 is up
// z axis: -1 is into the monitor, +1 is toward me
void SetView_CameraAtPoint_LookingAtAngle(
  float * xyzCameraPosition, float leftRightAngle, float upDownAngle);

void SetView_CameraLookingAtPoint_FromDistance_AtAngle(
  float * xyzFocalPoint, float distanceFromFocalPoint, float leftRightAngle, float upDownAngle);

#endif