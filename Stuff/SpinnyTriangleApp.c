#include "SpinnyTriangleApp.h"

#include "Utils.h"
#include "MsCounter.h"
#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "Resources.h"
#include "CommonApp.h"
#include "FatalErrorHandler.h"

void DrawToScreen(SpinnyTriangleApp_State * state);
void SetupWorldView(SpinnyTriangleApp_State * state);

void SpinnyTriangleApp_Initialize(SpinnyTriangleApp_State * state)
{
  memset(state, 0, sizeof(SpinnyTriangleApp_State));
  MsCounter_Init(&state->ElapsedTime);
  MsCounter_Reset(&state->ElapsedTime);

  // Initialize the main window w/ dorky smiley face icon
  state->MainWindow = CreateMainWindow("Nate Commander", Resource_MainWindowIcon_FileName, 0);
}

void SpinnyTriangleApp_HandleEvent(SpinnyTriangleApp_State * state, SDL_Event * sdlEvent)
{
  // TODO: window management, input handling, others?

  switch (sdlEvent->type)
  {
    case SDL_KEYDOWN:
      switch( sdlEvent->key.keysym.sym )
      {
        case SDLK_ESCAPE:
          SDL_Quit();
          // TODO: do we need to exit here?
          exit(0);
          break;

        case SDLK_SPACE:
          state->ShouldRotate = !state->ShouldRotate;
          break;

        case SDLK_a:
          Nate_Assert(state == 0, "oh noes an assertion failed");
      }
      break;

    case SDL_QUIT:
      exit(0);
      break;
  }
}

void SpinnyTriangleApp_Process(SpinnyTriangleApp_State * state)
{
  Uint64 msCount;

  if (state->ShouldRotate)
  {
    // update our sense of time
    msCount = MsCounter_Update(&state->ElapsedTime);

    // Produce a full rotation every 2 seconds
    state->CurrentAngle = (double)(msCount % 2000) * 360 / 2000;
  }
  else
  {
    // maintain a frozen sense of time
    MsCounter_ResetToCurrentCount(&state->ElapsedTime);
  }
}

void SpinnyTriangleApp_Draw(SpinnyTriangleApp_State * state)
{
  // Fools! They thought it would be sufficient to call this only once at the
  // start of the application!
  // At this point, we should have a properly set-up
  // double-buffered window for use with OpenGL.
  SetupWorldView(state);

  // Draw stuff
  DrawToScreen(state);
}

void SetupWorldView(SpinnyTriangleApp_State * state)
{
  float ratio;
  int width;
  int height;

  // get window width and height
  SDL_GetWindowSize(state->MainWindow.Window, &width, &height);
  ratio = (float)width / (float)height;

  // Our shading model--Gouraud (smooth).
  glShadeModel( GL_SMOOTH );

  // Culling.
  glCullFace( GL_BACK );
  glFrontFace( GL_CCW );
  glEnable( GL_CULL_FACE );

  // Set the clear color.
  glClearColor( 0, 0, 0, 0 );

  // Setup our viewport.
  glViewport( 0, 0, width, height );

  // Change to the projection matrix and set
  // our viewing volume.
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity( );

  // EXERCISE:
  // Replace this with a call to glFrustum.
  gluPerspective( 60.0, ratio, 1.0, 1024.0 );
}

void DrawToScreen(SpinnyTriangleApp_State * state)
{
  float angle;

  // EXERCISE:
  // Replace this awful mess with vertex
  // arrays and a call to glDrawElements.
  //
  // EXERCISE:
  // After completing the above, change
  // it to use compiled vertex arrays.
  //
  // EXERCISE:
  // Verify my windings are correct here ;).
  static GLfloat v0[] = { -1.0f, -1.0f,  1.0f };
  static GLfloat v1[] = {  1.0f, -1.0f,  1.0f };
  static GLfloat v2[] = {  1.0f,  1.0f,  1.0f };
  static GLfloat v3[] = { -1.0f,  1.0f,  1.0f };
  static GLfloat v4[] = { -1.0f, -1.0f, -1.0f };
  static GLfloat v5[] = {  1.0f, -1.0f, -1.0f };
  static GLfloat v6[] = {  1.0f,  1.0f, -1.0f };
  static GLfloat v7[] = { -1.0f,  1.0f, -1.0f };
  static GLubyte red[]    = { 255,   0,   0, 255 };
  static GLubyte green[]  = {   0, 255,   0, 255 };
  static GLubyte blue[]   = {   0,   0, 255, 255 };
  static GLubyte white[]  = { 255, 255, 255, 255 };
  static GLubyte yellow[] = {   0, 255, 255, 255 };
  static GLubyte black[]  = {   0,   0,   0, 255 };
  static GLubyte orange[] = { 255, 255,   0, 255 };
  static GLubyte purple[] = { 255,   0, 255,   0 };

  // Clear the color and depth buffers.
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  // We don't want to modify the projection matrix.
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity( );

  // Move down the z-axis. TODO: why?
  glTranslatef( 0.0, 0.0, -5.0 );

  // Rotate.
  angle = (float)state->CurrentAngle;
  glRotated( angle, 0.0, 1.0, 0.0 );

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
  glColor4ubv( black );
  glVertex3fv( v5 );
  glColor4ubv( orange );
  glVertex3fv( v6 );

  glColor4ubv( green );
  glVertex3fv( v1 );
  glColor4ubv( orange );
  glVertex3fv( v6 );
  glColor4ubv( blue );
  glVertex3fv( v2 );

  glColor4ubv( black );
  glVertex3fv( v5 );
  glColor4ubv( yellow );
  glVertex3fv( v4 );
  glColor4ubv( purple );
  glVertex3fv( v7 );

  glColor4ubv( black );
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
  glColor4ubv( black );
  glVertex3fv( v5 );

  glEnd( );

  // EXERCISE:
  // Draw text telling the user that 'Spc'
  // pauses the rotation and 'Esc' quits.
  // Do it using vetors and textured quads.

  // Swap the buffers. This this tells the driver to
  // render the next frame from the contents of the
  // back-buffer, and to set all rendering operations
  // to occur on what was the front-buffer.
  //
  // Double buffering prevents nasty visual tearing
  // from the application drawing on areas of the
  // screen that are being updated at the same time.
  SDL_GL_SwapWindow(state->MainWindow.Window);
}