#ifndef NATE_COMMANDER_BOX_GRAPHICS_H
#define NATE_COMMANDER_BOX_GRAPHICS_H

#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

extern const GLubyte red[4];
extern const GLubyte green[4];
extern const GLubyte blue[4];
extern const GLubyte white[4];
extern const GLubyte yellow[4];
extern const GLubyte black[4];
extern const GLubyte orange[4];
extern const GLubyte purple[4];
extern const GLubyte * colors[8];

void DrawAxisLineX(void);
void DrawAxisLineY(void);
void DrawAxisLineZ(void);
void DrawYAngledCube(float currentAngle);
void DrawSizedLocatedBox(float * whlDimensions, float * xyzTranslation);

#endif