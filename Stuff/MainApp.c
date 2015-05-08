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
#include "BoxGraphics.h"

struct MainApp_State
{
  WindowAndOpenGlContext MainWindow;
  lua_State * luaState;
};

void SetupWorldView(MainApp_State * state, lua_Number viewAngleX, lua_Number viewAngleY);

void DrawFloor();
void DrawFallingCubes();

void* MyLuaAlloc(void *ud, void *ptr, size_t osize, size_t nsize);
void LoadAndRunLuaFile(lua_State * state, const char * luaFileName);
void GetLuaMethod(lua_State * state, const char * luaFileName);

const char * NateCommander_LuaGameState = "NateCommander_LuaGameState";


// Global variables for Tokamak 
cneSimulator *gSim = NULL; 
// The number of cubes to render in the simulation (try values between 2 and about 50) 
#define CUBECOUNT 5 
cneRigidBody *gCubes[CUBECOUNT]; 
cneAnimatedBody *gFloor;
//lua_Number gFloorZOffset; // TODO: just let the simulator keep track of this? don't need a variable?
#define FLOOR_WIDTH 3
#define FLOOR_HEIGHT 0.3f
#define FLOOR_LENGTH 3
float floorDimensions[] = { FLOOR_WIDTH, FLOOR_HEIGHT, FLOOR_LENGTH };
float floorLocation[] = { 0, -3, 0 };
float cameraPosition[] = { 3, 3, 15 };
#define FLOOR_X 0
#define FLOOR_Y -3
#define FLOOR_Z 0

// Initialise the Tokamak physics engine. 
// Here's where the interesting stuff starts. 
void InitPhysics(void) 
{ 
  cneGeometry *geom;
  cneV3 inertiaTensor;
  cneV3 boxSize1;
  cneV3 floorSize1;
  cneV3 gravity;
  cneV3 pos;
  f32 mass;
  cneSimulatorSizeInfo sizeInfo;
  int i;
  s32 totalBody;

  // init stack-local structs
  cneV3_Init(&gravity);
  cneV3_Init(&boxSize1);
  cneV3_Init(&floorSize1);
  cneV3_Init(&pos);
  cneV3_Init(&inertiaTensor);

  // Create and initialise the simulator
  // Tell the simulator how many rigid bodies we have
  sizeInfo.rigidBodiesCount = CUBECOUNT;
  // Tell the simulator how many animated bodies we have
  sizeInfo.animatedBodiesCount = 1;
  // Tell the simulator how many bodies we have in total
  totalBody = sizeInfo.rigidBodiesCount + sizeInfo.animatedBodiesCount;
  sizeInfo.geometriesCount = totalBody;
  // The overlapped pairs count defines how many bodies it is possible to be in collision
  // at a single time. The SDK states this should be calculated as:
  // bodies * (bodies-1) / 2
  // So we'll take its word for it. :-)
  sizeInfo.overlappedPairsCount = totalBody * (totalBody - 1) / 2;
  // We're not using any of these so set them all to zero
  sizeInfo.rigidParticleCount = 0;
  sizeInfo.constraintsCount = 0;
  sizeInfo.terrainNodesStartCount = 0;
  
  // Set the gravity. Try changing this to see the effect on the objects
  cneV3_Set(&gravity, 0.0f, -10.0f, 0.0f);

  // Ready to go, create the simulator object
  gSim = cneSimulator_CreateSimulator(&sizeInfo, 0, &gravity);

  // Create rigid bodies for the cubes
  for (i=0; i<CUBECOUNT; i++)
  { 
    // Create a rigid body
    gCubes[i] = cneSimulator_CreateRigidBody(gSim);

    // Add geometry to the body and set it to be a box of dimensions 1, 1, 1
    geom = cneRigidBody_AddGeometry(gCubes[i]);

    cneV3_Set(&boxSize1, 1.0f, 1.0f, 1.0f);
    //geom->SetBoxSize(boxSize1[0], boxSize1[1], boxSize1[2]);
    cneGeometry_SetBoxSizeV3(geom, &boxSize1);
    // Update the bounding info of the object -- must always call this
    // after changing a body's geometry.
    cneRigidBody_UpdateBoundingInfo(gCubes[i]);

    // Set other properties of the object (mass, position, etc.)
    mass = 1.0f;
    inertiaTensor = cneBoxInertiaTensorV3(&boxSize1, mass);
    cneRigidBody_SetInertiaTensorV3(gCubes[i], &inertiaTensor);
    cneRigidBody_SetMass(gCubes[i], mass);

    // Vary the position so the cubes don't all exactly stack on top of each other
    cneV3_Set(&pos,
      (float)(rand()%10) / 100,
      4.0f + i*2.0f,
      (float)(rand()%10) / 100);

    cneRigidBody_SetPos(gCubes[i], &pos);
  }

  // Create an animated body for the floor
  gFloor = cneSimulator_CreateAnimatedBody(gSim);
  // Add geometry to the floor and set it to be a box with size as defined by the FLOORSIZE constant
  geom = cneAnimatedBody_AddGeometry(gFloor);
  cneV3_Set(&floorSize1, FLOOR_WIDTH, FLOOR_HEIGHT, FLOOR_LENGTH);
  cneGeometry_SetBoxSizeV3(geom, &floorSize1);
  cneAnimatedBody_UpdateBoundingInfo(gFloor);
  // Set the position of the floor within the simulator
  cneV3_Set(&pos, FLOOR_X, FLOOR_Y, FLOOR_Z);
  cneAnimatedBody_SetPos(gFloor, &pos);
  // All done
}


void KillPhysics(void)
{
  if (gSim)
  {
    // Destroy the simulator.
    // Note that this will release all related resources that we've allocated.
    cneSimulator_DestroySimulator(gSim);
    gSim = NULL;
  }
}


void MainApp_Initialize(MainApp_State ** state2)
{
  MainApp_State * state;
  cneSimulatorSizeInfo * sizeInfo;

  // create and destroy a tokamak thing - proof that it works
  sizeInfo = cneSimulatorSizeInfo_Create();
  cneSimulatorSizeInfo_Destroy(&sizeInfo);

  // Not calling this for today... tokamak is kinda crashing right now
  //InitPhysics();

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

void PrepareToCallLua_HandleEvent(MainApp_State * state, SDL_Event * sdlEvent)
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
}

void MainApp_HandleEvent(MainApp_State * state, SDL_Event * sdlEvent)
{
  int hasLuaEvent = 0;
  switch (sdlEvent->type)
  {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      PrepareToCallLua_HandleEvent(state, sdlEvent);
      hasLuaEvent = 1;
      // table.keySym = the pressed key
      lua_pushstring(state->luaState, "keySym");
      lua_pushnumber(state->luaState, sdlEvent->key.keysym.sym);
      lua_settable(state->luaState, -3);
      break;

    case SDL_MOUSEMOTION:
      PrepareToCallLua_HandleEvent(state, sdlEvent);
      hasLuaEvent = 1;
      // table.xrel = relative x movement
      lua_pushstring(state->luaState, "xrel");
      lua_pushnumber(state->luaState, sdlEvent->motion.xrel);
      lua_settable(state->luaState, -3);
      // table.yrel = relative y movement
      lua_pushstring(state->luaState, "yrel");
      lua_pushnumber(state->luaState, sdlEvent->motion.yrel);
      lua_settable(state->luaState, -3);
      // table.x = absolute x position
      lua_pushstring(state->luaState, "x");
      lua_pushnumber(state->luaState, sdlEvent->motion.x);
      lua_settable(state->luaState, -3);
      // table.y = absolute x position
      lua_pushstring(state->luaState, "y");
      lua_pushnumber(state->luaState, sdlEvent->motion.y);
      lua_settable(state->luaState, -3);
      break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEWHEEL:
      //sdlEvent->motion.xrel
      break;
      
    case SDL_QUIT:
      exit(0);
      break;
  }

  if (hasLuaEvent)
  {
    // call the method
    if (LUA_OK != lua_pcall(state->luaState, 2, 0, 0))
    {
      FatalError2("Failed to run lua MainApp_HandleEvent", lua_tolstring(state->luaState, -1, 0));
    }
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

  // note - LUA script causes the simulation to advance at the right time
  // this code was suggested to add some regulated randomness to it. (meh)
  /*
  // Do something to the physics state
  float fElapsed;
  static float fLastElapsed;
  // Find out how much time has elapsed since we were last called
  fElapsed = GetElapsedTime();
  // Prevent the elapsed time from being more than 20% greater or
  // less than the previous elapsed time.
  if (fLastElapsed != 0)
  {
    if (fElapsed > fLastElapsed * 1.2f)
      fElapsed = fLastElapsed * 1.2f;
    
    if (fElapsed < fLastElapsed * 0.8f)
      fElapsed = fLastElapsed * 0.8f;
  }
  // Stop the elapsed time from exceeding 1/45th of a second.
  if (fElapsed > 1.0f / 45.0f)
    fElapsed = 1.0f / 45.0f;
  // Store the elapsed time so that we can use it the next time around
  fLastElapsed = fElapsed;
  */
}

// LUA script calls this
void MainApp_AdvanceGSIM(lua_Number floorZOffset)
{
  // move the floor
  floorLocation[2] = (float)floorZOffset;

  // TODO: reposition any errant boxes

  // Not calling this for today... tokamak is still pretty unstable now
  //cneSimulator_Advance(gSim, 1.0f / 60.0f, 1, 0);
}

void MainApp_Draw(MainApp_State * state)
{
  /*
  MainApp_Draw(MainApp_State * state);
  */  
  lua_Number spinnyCubeAngle;
  lua_Number viewAngleX;
  lua_Number viewAngleY;

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
  if (LUA_OK != lua_pcall(state->luaState, 1, 3, 0))
  {
    FatalError("Failed to execute lua MainApp_Draw");
  }

  // the return values are the spinny cube angle and the view angle
  spinnyCubeAngle = lua_tonumber(state->luaState, -3);
  viewAngleX = lua_tonumber(state->luaState, -2);
  viewAngleY = lua_tonumber(state->luaState, -1);
  // and we're responsible for cleaning up returned values off the stack
  lua_pop(state->luaState, 3);

  // Fools! They thought it would be sufficient to call this only once at the
  // start of the application!
  // At this point, we should have a properly set-up
  // double-buffered window for use with OpenGL.
  SetupWorldView(state, viewAngleX, viewAngleY);

  // Clear the color and depth buffers.
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  // Draw stuff
  DrawAxisLineX();
  DrawAxisLineY();
  DrawAxisLineZ();
  DrawYAngledCube((float)spinnyCubeAngle);
  DrawSizedLocatedBox(floorDimensions, floorLocation);
  DrawFallingCubes();

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

void SetView_CameraAtAngle_LookingAtPoint_FromDistance(
  float leftRightAngle, float upDownAngle, 
  float * xyzFocalPoint, float distance)
{
  // TODO: this method is not tested at all

  // VERY FIRST do the distance thing
  glTranslatef(0.0f, 0.0f, distance);
  
  // then rotate and translate to the desired point
  SetView_CameraAtPoint_LookingAtAngle(xyzFocalPoint, leftRightAngle, upDownAngle);
}

void SetupWorldView(MainApp_State * state, lua_Number viewAngleX, lua_Number viewAngleY)
{
  float ratio;
  int width;
  int height;
  //float cameraDistance = 15.0f;
  //float focusPointX = 0.0f;
  //float focusPointY = 5.0f;
  //float focusPointZ = 0.0f;
  
  // get window width and height
  SDL_GetWindowSize(state->MainWindow.Window, &width, &height);
  if (width <= 0) width = 1;
  if (height <= 0) height = 1;
  ratio = (float)width / (float)height;

  // Our shading model--Gouraud (smooth).
  glShadeModel( GL_SMOOTH );

  // Culling.
  glCullFace( GL_BACK );
  glFrontFace( GL_CCW );
  glEnable( GL_CULL_FACE );

  // Depth testing (so I don't have to try to order my opaque objects)
  glEnable( GL_DEPTH_TEST );

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
  // TODO: NeHe used 45.0 degrees... what do I think looks good? (What did Halo use? I hated that)
  gluPerspective( 60.0, ratio, 1.0, 1024.0 );

  SetView_CameraAtPoint_LookingAtAngle(
    cameraPosition, 
    // translate viewing angle from screen coordinates (top left is 0,0 bottom right is width,height)
    // to angles between -360 and +360, and flip y so when user moves mouse up it's considered y+
    (((float)viewAngleX - (width * 0.5f)) / (width * 0.5f)) * 360.0f, 
    -(((float)viewAngleY - (height * 0.5f)) / (height * 0.5f)) * 360.0f);
  /*
  // first (TODO: why first? why must rotation be done before translation?)
  // rotate all the world objects in the negative-as-desired direction 
  // to simulate camera angle.
  // viewAngleX means to rotate left-right, which is implemented as "around the y axis"
  glRotated(viewAngleX, 0.0f, 1.0f, 0.0f);
  // viewAngleY means to rotate up-down, which is implemented as "around the x axis"
  glRotated(viewAngleY, 1.0f, 0.0f, 0.0f);

  // next (TODO: why next? why can we translate in world units AFTER rotating?)
  // translate all the world objects in the negative-as-desired direction 
  // to simulate the camera position
  glTranslatef(5.0f, -5.0f, 0.0f);
  */
}





/*
void DrawFallingCube(float xOffset, float yOffset, float zOffset, float xRot, float yRot, float zRot)
{
}
*/

void DrawFallingCubes()
{
  /*
  cneGeometry * geom;
  cneT3 t;
  cneT3_Init(&t);

  for (int i = 0; i < CUBECOUNT; i++)
  {
    // Get the transformation matrix for this cube
    t = cneRigidBody_GetTransform(gCubes[i]);

    // TODO: transform our model coordinates I guess?

    //cneRigidBody_BeginIterateGeometry(gCubes[i]);
    //geom = cneRigidBody_GetNextGeometry(gCubes[i]);
    
    // Set the vertex stream for the cube 
    gD3DDevice->SetStreamSource(0,vbCube,sizeof(strVertex)); 
    // Draw the cubes 
    for (i=0; i<CUBECOUNT; i++)
    {
      // Get the transformation matrix for this cube
      t = gCubes[i]->GetTransform();
      // Transfer the values to a D3DMATRIX that we can pass to DirectX
      dxTrans = D3DXMATRIX(
        t.rot[0][0], t.rot[0][1], t.rot[0][2], 0.0f, 
        t.rot[1][0], t.rot[1][1], t.rot[1][2], 0.0f,
        t.rot[2][0], t.rot[2][1], t.rot[2][2], 0.0f,
        t.pos[0],t.pos[1], t.pos[2], 1.0f ); 
      // Set the world transformation so that we can draw the cube at the correct position
      gD3DDevice->SetTransform(D3DTS_WORLD, &dxTrans );
      // Render the cube 
      gD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST,0,12);
    }
  }
  */
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
  if (LUA_OK != luaL_loadbufferx(luaState, (char*)luaFileData, luaFileDataLength, luaFileName, "t"))
  {
    free(luaFileData);

    // lua pushes an error message onto the stack, let's show that to the user
    FatalError2("Failed to load/parse/first-time-interpret LUA chunk: ", lua_tolstring(luaState, -1, 0));
  }
  free(luaFileData);
  if (LUA_OK != lua_pcall(luaState, 0, 0, 0))
  {
    FatalError("Failed to execute LUA chunk");
  }
}