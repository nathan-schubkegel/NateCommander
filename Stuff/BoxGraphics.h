#ifndef NATE_COMMANDER_BOX_GRAPHICS_H
#define NATE_COMMANDER_BOX_GRAPHICS_H

void DrawAxisLineX(void);
void DrawAxisLineY(void);
void DrawAxisLineZ(void);
void DrawYAngledCube(float currentAngle);
void DrawSizedLocatedBox(float * whlDimensions, float * xyzTranslation);

#endif