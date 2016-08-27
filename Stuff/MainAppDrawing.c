/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "MainAppDrawing.h"

#include "MainAppLua.h"
#include <Windows.h>
#include <GL\gl.h>
#include <GL\glu.h>
#include "BoxGraphics.h"
#include "ViewGraphics.h"
#include "NateMashDrawing.h"

void MainAppDrawing_Draw(MainAppHostStruct * hostStruct)
{
  int width, height;

  // get window width and height
  SDL_GetWindowSize(hostStruct->MainWindow.Window, &width, &height);

  // make them so divide-by-zero errors will certainly not occur
  // when lua code divides these to determine an aspect ratio later
  if (width <= 0) width = 1;
  if (height <= 0) height = 1;

  /////////////////////////////////////////////////////////////
  //
  // TODO: how much from this point down should be left to lua?
  //
  /////////////////////////////////////////////////////////////

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

  // call the lua function to draw whatever is desired
  MainAppLua_CallDraw(hostStruct, width, height);

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