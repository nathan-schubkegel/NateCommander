#include "MainAppDrawing.h"

#include "MainAppLua.h"
#include <Windows.h>
#include <GL\gl.h>
#include <GL\glu.h>
#include "BoxGraphics.h"
#include "ViewGraphics.h"

// TODO: make these less global
// The number of cubes to render in the simulation (try values between 2 and about 50) 
//#define CUBECOUNT 5 
// coordinates are (x, y, z) or (width, height length)
float floorDimensions[] = { 3, 0.3f, 3};
float floorLocation[] = { 0, -3, 0 };
float cameraPosition[] = { 3, 3, 15 };

void MySetupView(MainAppHostStruct * hostStruct, lua_Number viewAngleX, lua_Number viewAngleY);

void MainAppDrawing_Draw(MainAppHostStruct * hostStruct)
{
  lua_Number spinnyCubeAngle;
  lua_Number floorZOffset;
  lua_Number viewAngleX;
  lua_Number viewAngleY;

  // call the lua function to get needed drawing info
  MainAppLua_CallDraw(hostStruct, &spinnyCubeAngle, &floorZOffset, &viewAngleX, &viewAngleY);
  floorLocation[2] = (float)floorZOffset;

  // setup view matrix
  MySetupView(hostStruct, viewAngleX, viewAngleY);

  // Clear the color and depth buffers.
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  // Draw stuff
  DrawAxisLineX();
  DrawAxisLineY();
  DrawAxisLineZ();
  DrawYAngledCube((float)spinnyCubeAngle);
  DrawSizedLocatedBox(floorDimensions, floorLocation);

  // Swap the buffers. This this tells the driver to
  // render the next frame from the contents of the
  // back-buffer, and to set all rendering operations
  // to occur on what was the front-buffer.
  //
  // Double buffering prevents nasty visual tearing
  // from the application drawing on areas of the
  // screen that are being updated at the same time.
  SDL_GL_SwapWindow(hostStruct->MainWindow.Window);
}

void MySetupView(MainAppHostStruct * hostStruct, lua_Number viewAngleX, lua_Number viewAngleY)
{
  float ratio;
  int width;
  int height;
  //float cameraDistance = 15.0f;
  //float focusPointX = 0.0f;
  //float focusPointY = 5.0f;
  //float focusPointZ = 0.0f;
  
  // get window width and height
  SDL_GetWindowSize(hostStruct->MainWindow.Window, &width, &height);
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