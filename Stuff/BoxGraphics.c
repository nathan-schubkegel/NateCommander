#include "BoxGraphics.h"

#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

const GLfloat cubeWidth = 2.0f;
const GLfloat cubeHeight = 2.0f;
const GLfloat cubeLength = 2.0f;
const GLfloat v0[] = { -1.0f, -1.0f,  1.0f }; // red
const GLfloat v1[] = {  1.0f, -1.0f,  1.0f }; // green
const GLfloat v2[] = {  1.0f,  1.0f,  1.0f }; // blue
const GLfloat v3[] = { -1.0f,  1.0f,  1.0f }; // white
const GLfloat v4[] = { -1.0f, -1.0f, -1.0f }; // yellow
const GLfloat v5[] = {  1.0f, -1.0f, -1.0f }; // brown
const GLfloat v6[] = {  1.0f,  1.0f, -1.0f }; // orange
const GLfloat v7[] = { -1.0f,  1.0f, -1.0f }; // purple
const GLubyte red[]    = { 255,   0,   0, 255 };
const GLubyte green[]  = {   0, 255,   0, 255 };
const GLubyte blue[]   = {   0,   0, 255, 255 };
const GLubyte white[]  = { 255, 255, 255, 255 };
const GLubyte yellow[] = {   0, 255, 255, 255 };
const GLubyte brown[]  = {  80,  50,  50, 255 };
const GLubyte orange[] = { 255, 255,   0, 255 };
const GLubyte purple[] = { 255,   0, 255,   0 };
const GLubyte * colors[] = { red, green, blue, white, yellow, brown, orange, purple };

// -x = orange
// +x = red
void DrawAxisLineX(void)
{
  glPushMatrix();

  // scale so the axis box is freaking long and thin
  glScalef(
    20.0f, 
    0.05f,
    0.05f);

  // Send our triangle data to the pipeline.
  glBegin( GL_TRIANGLES );

  glColor4ubv( orange );
  glVertex3fv( v0 );
  glColor4ubv( red );
  glVertex3fv( v1 );
  glColor4ubv( red );
  glVertex3fv( v2 );

  glColor4ubv( orange );
  glVertex3fv( v0 );
  glColor4ubv( red );
  glVertex3fv( v2 );
  glColor4ubv( orange );
  glVertex3fv( v3 );

  glColor4ubv( red );
  glVertex3fv( v1 );
  glColor4ubv( red );
  glVertex3fv( v5 );
  glColor4ubv( red );
  glVertex3fv( v6 );

  glColor4ubv( red );
  glVertex3fv( v1 );
  glColor4ubv( red );
  glVertex3fv( v6 );
  glColor4ubv( red );
  glVertex3fv( v2 );

  glColor4ubv( red );
  glVertex3fv( v5 );
  glColor4ubv( orange );
  glVertex3fv( v4 );
  glColor4ubv( orange );
  glVertex3fv( v7 );

  glColor4ubv( red );
  glVertex3fv( v5 );
  glColor4ubv( orange );
  glVertex3fv( v7 );
  glColor4ubv( red );
  glVertex3fv( v6 );

  glColor4ubv( orange );
  glVertex3fv( v4 );
  glColor4ubv( orange );
  glVertex3fv( v0 );
  glColor4ubv( orange );
  glVertex3fv( v3 );

  glColor4ubv( orange );
  glVertex3fv( v4 );
  glColor4ubv( orange );
  glVertex3fv( v3 );
  glColor4ubv( orange );
  glVertex3fv( v7 );

  glColor4ubv( orange );
  glVertex3fv( v3 );
  glColor4ubv( red );
  glVertex3fv( v2 );
  glColor4ubv( red );
  glVertex3fv( v6 );

  glColor4ubv( orange );
  glVertex3fv( v3 );
  glColor4ubv( red );
  glVertex3fv( v6 );
  glColor4ubv( orange );
  glVertex3fv( v7 );

  glColor4ubv( red );
  glVertex3fv( v1 );
  glColor4ubv( orange );
  glVertex3fv( v0 );
  glColor4ubv( orange );
  glVertex3fv( v4 );

  glColor4ubv( red );
  glVertex3fv( v1 );
  glColor4ubv( orange );
  glVertex3fv( v4 );
  glColor4ubv( red );
  glVertex3fv( v5 );

  glEnd( );

  glPopMatrix();
}

// -y = white
// +y = green
void DrawAxisLineY(void)
{
  glPushMatrix();

  // scale so the axis box is freaking long and thin
  glScalef(
    0.05f, 
    20.0f,
    0.05f);

  // Send our triangle data to the pipeline.
  glBegin( GL_TRIANGLES );

  glColor4ubv( white );
  glVertex3fv( v0 );
  glColor4ubv( white );
  glVertex3fv( v1 );
  glColor4ubv( green );
  glVertex3fv( v2 );

  glColor4ubv( white );
  glVertex3fv( v0 );
  glColor4ubv( green );
  glVertex3fv( v2 );
  glColor4ubv( green );
  glVertex3fv( v3 );

  glColor4ubv( white );
  glVertex3fv( v1 );
  glColor4ubv( white );
  glVertex3fv( v5 );
  glColor4ubv( green );
  glVertex3fv( v6 );

  glColor4ubv( white );
  glVertex3fv( v1 );
  glColor4ubv( green );
  glVertex3fv( v6 );
  glColor4ubv( green );
  glVertex3fv( v2 );

  glColor4ubv( white );
  glVertex3fv( v5 );
  glColor4ubv( white );
  glVertex3fv( v4 );
  glColor4ubv( green );
  glVertex3fv( v7 );

  glColor4ubv( white );
  glVertex3fv( v5 );
  glColor4ubv( white );
  glVertex3fv( v7 );
  glColor4ubv( green );
  glVertex3fv( v6 );

  glColor4ubv( white );
  glVertex3fv( v4 );
  glColor4ubv( white );
  glVertex3fv( v0 );
  glColor4ubv( green );
  glVertex3fv( v3 );

  glColor4ubv( white );
  glVertex3fv( v4 );
  glColor4ubv( green );
  glVertex3fv( v3 );
  glColor4ubv( green );
  glVertex3fv( v7 );

  glColor4ubv( green );
  glVertex3fv( v3 );
  glColor4ubv( green );
  glVertex3fv( v2 );
  glColor4ubv( green );
  glVertex3fv( v6 );

  glColor4ubv( green );
  glVertex3fv( v3 );
  glColor4ubv( green );
  glVertex3fv( v6 );
  glColor4ubv( green );
  glVertex3fv( v7 );

  glColor4ubv( white );
  glVertex3fv( v1 );
  glColor4ubv( white );
  glVertex3fv( v0 );
  glColor4ubv( white );
  glVertex3fv( v4 );

  glColor4ubv( white );
  glVertex3fv( v1 );
  glColor4ubv( white );
  glVertex3fv( v4 );
  glColor4ubv( white );
  glVertex3fv( v5 );

  glEnd( );

  glPopMatrix();
}

// +z = blue
// -z = purple
void DrawAxisLineZ(void)
{
  glPushMatrix();

  // scale so the axis box is freaking long and thin
  glScalef(
    0.05f, 
    0.05f,
    20.0f);

  // Send our triangle data to the pipeline.
  glBegin( GL_TRIANGLES );

  glColor4ubv( blue );
  glVertex3fv( v0 );
  glColor4ubv( blue );
  glVertex3fv( v1 );
  glColor4ubv( blue );
  glVertex3fv( v2 );

  glColor4ubv( blue );
  glVertex3fv( v0 );
  glColor4ubv( blue );
  glVertex3fv( v2 );
  glColor4ubv( blue );
  glVertex3fv( v3 );

  glColor4ubv( blue );
  glVertex3fv( v1 );
  glColor4ubv( purple );
  glVertex3fv( v5 );
  glColor4ubv( purple );
  glVertex3fv( v6 );

  glColor4ubv( blue );
  glVertex3fv( v1 );
  glColor4ubv( purple );
  glVertex3fv( v6 );
  glColor4ubv( blue );
  glVertex3fv( v2 );

  glColor4ubv( purple );
  glVertex3fv( v5 );
  glColor4ubv( purple );
  glVertex3fv( v4 );
  glColor4ubv( purple );
  glVertex3fv( v7 );

  glColor4ubv( purple );
  glVertex3fv( v5 );
  glColor4ubv( purple );
  glVertex3fv( v7 );
  glColor4ubv( purple );
  glVertex3fv( v6 );

  glColor4ubv( purple );
  glVertex3fv( v4 );
  glColor4ubv( blue );
  glVertex3fv( v0 );
  glColor4ubv( blue );
  glVertex3fv( v3 );

  glColor4ubv( purple );
  glVertex3fv( v4 );
  glColor4ubv( blue );
  glVertex3fv( v3 );
  glColor4ubv( purple );
  glVertex3fv( v7 );

  glColor4ubv( blue );
  glVertex3fv( v3 );
  glColor4ubv( blue );
  glVertex3fv( v2 );
  glColor4ubv( purple );
  glVertex3fv( v6 );

  glColor4ubv( blue );
  glVertex3fv( v3 );
  glColor4ubv( purple );
  glVertex3fv( v6 );
  glColor4ubv( purple );
  glVertex3fv( v7 );

  glColor4ubv( blue );
  glVertex3fv( v1 );
  glColor4ubv( blue );
  glVertex3fv( v0 );
  glColor4ubv( purple );
  glVertex3fv( v4 );

  glColor4ubv( blue );
  glVertex3fv( v1 );
  glColor4ubv( purple );
  glVertex3fv( v4 );
  glColor4ubv( purple );
  glVertex3fv( v5 );

  glEnd( );

  glPopMatrix();
}

void MyDrawRainbowCube(void)
{
  // Send our triangle data to the pipeline.
  glBegin( GL_TRIANGLES );

  glColor4ubv( red );
  glVertex3fv( v0 );
  glColor4ubv( green );
  glVertex3fv( v1 );
  glColor4ubv( blue );
  glVertex3fv( v2 );

  glColor4ubv( red );
  glVertex3fv( v0 );
  glColor4ubv( blue );
  glVertex3fv( v2 );
  glColor4ubv( white );
  glVertex3fv( v3 );

  glColor4ubv( green );
  glVertex3fv( v1 );
  glColor4ubv( brown );
  glVertex3fv( v5 );
  glColor4ubv( orange );
  glVertex3fv( v6 );

  glColor4ubv( green );
  glVertex3fv( v1 );
  glColor4ubv( orange );
  glVertex3fv( v6 );
  glColor4ubv( blue );
  glVertex3fv( v2 );

  glColor4ubv( brown );
  glVertex3fv( v5 );
  glColor4ubv( yellow );
  glVertex3fv( v4 );
  glColor4ubv( purple );
  glVertex3fv( v7 );

  glColor4ubv( brown );
  glVertex3fv( v5 );
  glColor4ubv( purple );
  glVertex3fv( v7 );
  glColor4ubv( orange );
  glVertex3fv( v6 );

  glColor4ubv( yellow );
  glVertex3fv( v4 );
  glColor4ubv( red );
  glVertex3fv( v0 );
  glColor4ubv( white );
  glVertex3fv( v3 );

  glColor4ubv( yellow );
  glVertex3fv( v4 );
  glColor4ubv( white );
  glVertex3fv( v3 );
  glColor4ubv( purple );
  glVertex3fv( v7 );

  glColor4ubv( white );
  glVertex3fv( v3 );
  glColor4ubv( blue );
  glVertex3fv( v2 );
  glColor4ubv( orange );
  glVertex3fv( v6 );

  glColor4ubv( white );
  glVertex3fv( v3 );
  glColor4ubv( orange );
  glVertex3fv( v6 );
  glColor4ubv( purple );
  glVertex3fv( v7 );

  glColor4ubv( green );
  glVertex3fv( v1 );
  glColor4ubv( red );
  glVertex3fv( v0 );
  glColor4ubv( yellow );
  glVertex3fv( v4 );

  glColor4ubv( green );
  glVertex3fv( v1 );
  glColor4ubv( yellow );
  glVertex3fv( v4 );
  glColor4ubv( brown );
  glVertex3fv( v5 );

  glEnd( );
}

void DrawYAngledCube(float currentAngle)
{
  glPushMatrix();

  // Move down the z-axis. 
  // This makes the cube appear 5.0 further into the screen than where we're currently viewing from
  //glTranslatef( 0.0, 0.0, -5.0 );

  // Rotate.
  // the first parameter is in degrees
  // the later three values indicate the vector of the axis around which we'll rotate
  glRotated( currentAngle, 0.0, 1.0, 0.0 );

  MyDrawRainbowCube();

  glPopMatrix();
}

void DrawSizedLocatedBox(float * whlDimensions, float * xyzTranslation)
{
  glPushMatrix();

  // Move down the y-axis so the floor appears at the right height
  // Move up or down the z-axis based on where user specified the floor to be
  // TODO: Why do I have to translate first? (Why doesn't it work right when I scale first?)
  glTranslatef( xyzTranslation[0], xyzTranslation[1], xyzTranslation[2] );

  // scale so the floor is squat (or whatever dimensons the floor really is)
  glScalef(
    (1.0f / cubeWidth) * whlDimensions[0], 
    (1.0f / cubeHeight) * whlDimensions[1],
    (1.0f / cubeLength) * whlDimensions[2]);

  MyDrawRainbowCube();

  glPopMatrix();
}