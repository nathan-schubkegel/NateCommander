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
#include "LuaExports.h"
#include "ctokamak.h"

struct MainApp_State
{
  WindowAndOpenGlContext MainWindow;
  lua_State * luaState;
};

void DrawToScreen(MainApp_State * state, lua_Number currentAngle);
void SetupWorldView(MainApp_State * state);
void* MyLuaAlloc(void *ud, void *ptr, size_t osize, size_t nsize);
void LoadAndRunLuaFile(lua_State * state, const char * luaFileName);
void GetLuaMethod(lua_State * state, const char * luaFileName);

const char * NateCommander_LuaGameState = "NateCommander_LuaGameState";

void MainApp_Initialize(MainApp_State ** state2)
{
  MainApp_State * state;

  // create a tokamak thing? temporarily? proof that it works?
  cneSimulatorSizeInfo * sizeInfo;
  sizeInfo = cneSimulatorSizeInfo_Create();
  cneSimulatorSizeInfo_Destroy(&sizeInfo);

  state = malloc(sizeof(MainApp_State));
  if (state == 0) 
  {
    FatalError("failed to malloc for MainApp_State");
  }
  *state2 = state;
  memset(state, 0, sizeof(MainApp_State));

  // Initialize the main window w/ dorky smiley face icon
  state->MainWindow = CreateMainWindow("Nate Commander", Resource_MainWindowIcon_FileName, 0);
  
  // Initialize a LUA state
  state->luaState = lua_newstate(MyLuaAlloc, 0);
  if (state->luaState == 0) 
  {
    FatalError("Failed to create LUA state");
  }

  // add the exported C methods
  LuaExports_PublishCMethods(state->luaState);

  // Load the MainApp lua file
  LoadAndRunLuaFile(state->luaState, "MainApp.lua");

  // create a table for game state,
  // it will live in the registry as "NateCommander_LuaGameState"
  lua_pushstring(state->luaState, NateCommander_LuaGameState);
  lua_newtable(state->luaState);
  lua_settable(state->luaState, LUA_REGISTRYINDEX);

  // Get the 'MainApp_Initialize' method
  lua_getglobal(state->luaState, "MainApp_Initialize");
  if (lua_isnil(state->luaState, -1))
  {
    FatalError("Unable to find lua MainApp_Initialize");
  }

  // first argument is the game state object
  lua_pushstring(state->luaState, NateCommander_LuaGameState);
  lua_gettable(state->luaState, LUA_REGISTRYINDEX);  

  // call the method
  if (LUA_OK != lua_pcall(state->luaState, 1, 0, 0))
  {
    FatalError("Failed to run lua MainApp_Initialize");
  }
}

void MainApp_HandleEvent(MainApp_State * state, SDL_Event * sdlEvent)
{
  // Get the 'MainApp_HandleEvent' method
  lua_getglobal(state->luaState, "MainApp_HandleEvent");
  if (lua_isnil(state->luaState, -1))
  {
    FatalError("Unable to find lua MainApp_HandleEvent");
  }

  // first argument is the game state object
  lua_pushstring(state->luaState, NateCommander_LuaGameState);
  lua_gettable(state->luaState, LUA_REGISTRYINDEX);

  // second argument is a new table to hold the event info
  lua_newtable(state->luaState);

  // table.type = the sdl event type
  lua_pushstring(state->luaState, "type");
  lua_pushnumber(state->luaState, sdlEvent->type);
  lua_settable(state->luaState, -3);

  switch (sdlEvent->type)
  {
    case SDL_KEYDOWN:
      // table.keySymbol = the pressed key
      lua_pushstring(state->luaState, "keySym");
      lua_pushnumber(state->luaState, sdlEvent->key.keysym.sym);
      lua_settable(state->luaState, -3);
      break;
      
    case SDL_QUIT:
      exit(0);
      break;
  }

  // call the method
  if (LUA_OK != lua_pcall(state->luaState, 2, 0, 0))
  {
    FatalError("Failed to run lua MainApp_HandleEvent");
  }
}

void MainApp_Process(MainApp_State * state)
{
  // Get the 'MainApp_Process' method
  lua_getglobal(state->luaState, "MainApp_Process");
  if (lua_isnil(state->luaState, -1))
  {
    FatalError("Unable to find lua MainApp_Process");
  }

  // first argument is the game state object
  lua_pushstring(state->luaState, NateCommander_LuaGameState);
  lua_gettable(state->luaState, LUA_REGISTRYINDEX);

  // call the lua function
  if (LUA_OK != lua_pcall(state->luaState, 1, 0, 0))
  {
    FatalError("Failed to execute lua MainApp_Process");
  }
}

void MainApp_Draw(MainApp_State * state)
{
  /*
  MainApp_Draw(MainApp_State * state);
  */  
  lua_Number currentAngle;

  // Get the 'MainApp_Draw' method
  lua_getglobal(state->luaState, "MainApp_Draw");
  if (lua_isnil(state->luaState, -1))
  {
    FatalError("Unable to find lua MainApp_Draw");
  }

  // first argument is the game state object
  lua_pushstring(state->luaState, NateCommander_LuaGameState);
  lua_gettable(state->luaState, LUA_REGISTRYINDEX);

  // call the lua function
  if (LUA_OK != lua_pcall(state->luaState, 1, 1, 0))
  {
    FatalError("Failed to execute lua MainApp_Draw");
  }

  // the return value is the current angle to draw
  currentAngle = lua_tonumber(state->luaState, -1);
  lua_pop(state->luaState, 1);

  // Fools! They thought it would be sufficient to call this only once at the
  // start of the application!
  // At this point, we should have a properly set-up
  // double-buffered window for use with OpenGL.
  SetupWorldView(state);

  // Draw stuff
  DrawToScreen(state, currentAngle);
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

void DrawToScreen(MainApp_State * state, lua_Number currentAngle)
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
  angle = (float)currentAngle;
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

void LoadAndRunLuaFile(lua_State * luaState, const char * luaFileName)
{
  void* luaFileData;
  long luaFileDataLength;

  luaFileData = ResourceLoader_LoadLuaFile(luaFileName, &luaFileDataLength);
  if (luaFileData == 0)
  {
    FatalError("Failed to load LUA file contents");
  }
  
  // This could still be dorked up by unicode or whatever. Oh well. They'll learn.
  BuildAssertSize(sizeof(char), 1); // Char is supposed to be 1 byte for this to work
  if (LUA_OK != luaL_loadbufferx(luaState, (char*)luaFileData, luaFileDataLength, "my whatever", "t"))
  {
    free(luaFileData);
    FatalError("Failed to load LUA chunk");
  }
  free(luaFileData);
  if (LUA_OK != lua_pcall(luaState, 0, 0, 0))
  {
    FatalError("Failed to execute LUA chunk");
  }
}