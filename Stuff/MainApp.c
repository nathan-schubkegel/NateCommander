#include "MainApp.h"

#include "Utils.h"
#include "MsCounter.h"
#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "Resources.h"
#include "ResourceLoader.h"
#include "CommonApp.h"
#include "FatalErrorHandler.h"
#include "lua.h"
#include "lauxlib.h"

struct MainApp_State
{
  WindowAndOpenGlContext MainWindow;
  double CurrentAngle;
  MsCounter ElapsedTime;
  Uint8 ShouldRotate;
  lua_State * luaState;
};

void DrawToScreen(MainApp_State * state);
void SetupWorldView(MainApp_State * state);
void* MyLuaAlloc(void *ud, void *ptr, size_t osize, size_t nsize);

void MainApp_Initialize(MainApp_State ** state)
{
  MainApp_State * state2;
  char * luaFileData;
  long luaFileDataLength;
  const char * message;
  size_t messageLength;

  state2 = malloc(sizeof(MainApp_State));
  if (state2 == 0) FatalError("failed to malloc for MainApp_State");
  *state = state2;

  memset(state2, 0, sizeof(MainApp_State));
  MsCounter_Init(&state2->ElapsedTime);
  MsCounter_Reset(&state2->ElapsedTime);

  // Initialize the main window w/ dorky smiley face icon
  state2->MainWindow = CreateMainWindow("Nate Commander", Resource_MainWindowIcon_FileName, 0);
  
  // Initialize a LUA state
  state2->luaState = lua_newstate(MyLuaAlloc, 0);
  if (state2->luaState == 0) FatalError("Failed to create LUA state");

  luaFileData = ResourceLoader_LoadLuaFile("MainApp.lua", &luaFileDataLength);
  if (luaFileData == 0)
  {
    FatalError("Failed to load LUA file contents");
  }
   
  if (LUA_OK != luaL_loadbufferx(state2->luaState, luaFileData, luaFileDataLength, "my whatever", "t"))
  {
    FatalError("Failed to load LUA chunk");
  }
  free(luaFileData);

  // execute the LUA chunk just loaded
  if (LUA_OK != lua_pcall(state2->luaState, 0, 1, 0))
  {
    FatalError("Failed to execute LUA chunk");
  }

  // execute the LUA function that it returned
  if (LUA_OK != lua_pcall(state2->luaState, 0, 1, 0))
  {
    FatalError("Failed to execute method returned by LUA chunk");
  }

  // get the returned string
  message = lua_tolstring(state2->luaState, 1, &messageLength);
  if (message != 0)
  {
    NonFatalError(message);
  }
}

void MainApp_HandleEvent(MainApp_State * state, SDL_Event * sdlEvent)
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

void MainApp_Process(MainApp_State * state)
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

void MainApp_Draw(MainApp_State * state)
{
  // Fools! They thought it would be sufficient to call this only once at the
  // start of the application!
  // At this point, we should have a properly set-up
  // double-buffered window for use with OpenGL.
  SetupWorldView(state);

  // Draw stuff
  DrawToScreen(state);
}

void SetupWorldView(MainApp_State * state)
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

void DrawToScreen(MainApp_State * state)
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

/*
The lua allocator function must provide a functionality similar to realloc, 
but not exactly the same. Its arguments are 
  ud, an opaque pointer passed to lua_newstate; 
  ptr, a pointer to the block being allocated/reallocated/freed; 
  osize, the original size of the block or some code about what is being allocated; 
  nsize, the new size of the block.

When ptr is not NULL, osize is the size of the block pointed by ptr, that is, 
the size given when it was allocated or reallocated.
When ptr is NULL, osize encodes the kind of object that Lua is allocating. 
osize is any of LUA_TSTRING, LUA_TTABLE, LUA_TFUNCTION, LUA_TUSERDATA, or 
LUA_TTHREAD when (and only when) Lua is creating a new object of that type. 
When osize is some other value, Lua is allocating memory for something else.

Lua assumes the following behavior from the allocator function:
  When nsize is zero, the allocator should behave like free and return NULL.
  When nsize is not zero, the allocator should behave like realloc. 
  The allocator returns NULL if and only if it cannot fulfill the request. 
  Lua assumes that the allocator never fails when osize >= nsize.

Note that Standard C ensures that free(NULL) has no effect and 
that realloc(NULL, size) is equivalent to malloc(size). 
This code assumes that realloc does not fail when shrinking a block. 
(Although Standard C does not ensure this behavior, it seems to be a 
safe assumption.)
*/
void* MyLuaAlloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
  // eliminate compiler warning
  (void)ud;
  (void)osize;

  if (nsize == 0) 
  {
    free(ptr);
    return NULL;
  }
  else
  {
    return realloc(ptr, nsize);
  }
}
