/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "NateMashDrawing.h"

#include <Windows.h>
#include <GL\gl.h>
#include <GL\glu.h>
#include "BoxGraphics.h"
#include "FatalErrorHandler.h"

void MyDrawNode(NateMashNode * node)
{
  size_t i, j, k, iVert, iNormal;
  size_t count;
  size_t stride;
  size_t vertexInputIndex;
  size_t normalInputIndex;
  size_t numInputs;
  int vertIndex;
  int normalIndex;
  float vertParts[3];
  float normalParts[3];
  NateMashSource * vertSource;
  NateMashSource * normalSource;
  float * vertData;
  float * normalData;
  int * dataIndexes;
  NateMashGeometry * geometry;

  geometry = node->geometry;

  // adjust opengl transform by the amount required for this node
  glMultMatrixf(node->transform.elements);

  glBegin( GL_TRIANGLES );

  // find the "VERTEX" and "NORMAL" inputs
  // (I guess we just assume every mash has a single "VERTEX" input and single "NORMAL" input)
  vertexInputIndex = 0xFFFFFFFF;
  normalInputIndex = 0xFFFFFFFF;
  for (i = 0; i < geometry->polylist.numInputs; i++)
  {
    if (geometry->polylist.inputs[i].dataType == NateMash_DataType_Vertex)
    {
      NateCheck(vertexInputIndex == 0xFFFFFFFF, "there can be only one!");
      vertexInputIndex = i;
    }
    else if (geometry->polylist.inputs[i].dataType == NateMash_DataType_Normal)
    {
      NateCheck(normalInputIndex == 0xFFFFFFFF, "there can be only one!");
      normalInputIndex = i;
    }
  }
  NateCheck(vertexInputIndex != 0xFFFFFFFF, "there must be at least one!");
  NateCheck(normalInputIndex != 0xFFFFFFFF, "there must be at least one!");
  
  // get the source of vertex data
  // (I guess we just assume the source is using 3 floats to make a vertex)
  vertSource = geometry->polylist.inputs[vertexInputIndex].source;
  vertData = vertSource->data;
  NateAssert(vertSource->stride == 3, "we assume sources depict triangles");

  // get the source of normal data
  // (I guess we just assume the source is using 3 floats to make a vertex)
  normalSource = geometry->polylist.inputs[normalInputIndex].source;
  normalData = normalSource->data;
  NateAssert(normalSource->stride == 3, "we assume sources depict triangles");

  // iterate through data coordinates
  // (I guess we just assume every 3 of these is a triangle that needs to be drawn)
  count = geometry->polylist.numDataCoordinates;
  stride = 3 * geometry->polylist.numInputs;
  dataIndexes = geometry->polylist.dataIndexes;
  numInputs = geometry->polylist.numInputs;
  NateAssert(count * stride == geometry->polylist.numDataIndexes, "right?");
  for (i = 0; i < count; i++)
  {
    iVert = i * stride + vertexInputIndex; // get to index of first vertex data index for this triangle
    iNormal = i * stride + normalInputIndex; // same, for normal data

    if (node->material != 0 && node->material->effect->effectType == NateMash_EffectType_Phong)
    {
      // use the material's diffuse color
      glColor4fv(node->material->effect->data.phong.diffuse.rgba);
    }
    else 
    {
      // random-ish color
      glColor4ubv(colors[i % 8]);
    }

    // j = for each of the 3 vertices in a triangle
    for (j = 0; j < 3; j++)
    {
      vertIndex = dataIndexes[iVert + j * numInputs];
      normalIndex = dataIndexes[iNormal + j * numInputs];

      // k = for each of the 3 x,y,z parts of a vertex
      for (k = 0; k < 3; k++)
      {
        vertParts[k] = vertData[vertIndex * 3 + k];
        normalParts[k] = normalData[normalIndex * 3 + k];
      }

      // set the normal for this vertex
      glNormal3fv( normalParts );

      // draw this vertex
      glVertex3fv( vertParts );
    }
  }

  glEnd( );

  for (i = 0; i < node->nodes.numNodes; i++)
  {
    // TODO: could call glGetError() to make sure this succeeds
    // but I'm not sure how I want to handle GL errors across the board
    // (and if some sloppy earlier/other code caused a GL error, do I throw it away here first?)

    // preserve and restore the MODELVIEW matrix before/after drawing children
    glPushMatrix();
    MyDrawNode(&node->nodes.nodes[i]);
    glPopMatrix();
  }
}

void NateMash_Draw(NateMash * mash)
{
  size_t i;
  for (i = 0; i < mash->nodes.numNodes; i++)
  {
    // TODO: could call glGetError() to make sure this succeeds
    // but I'm not sure how I want to handle GL errors across the board
    // (and if some sloppy earlier/other code caused a GL error, do I throw it away here first?)

    // preserve and restore the MODELVIEW matrix before/after drawing children
    glPushMatrix();
    MyDrawNode(&mash->nodes.nodes[i]);
    glPopMatrix();
  }
}

void NateMash_DrawUpright(NateMash * mash, float * position, float * rotation, float * scale)
{
  glPushMatrix();

  // TODO: Why do I have to translate first? (Why doesn't it work right when I scale first?)
  if (position != 0)
  {
    glTranslatef( position[0], position[1], position[2] );
  }

  // scale so the floor is squat (or whatever dimensons the floor really is)
  if (scale != 0)
  {
    glScalef( scale[0], scale[1], scale[2] );
  }

  // rotate
  if (rotation != 0)
  {
    // left-right is implemented as "around the y axis"
    // up-down is implemented as "around the x axis"
    // upright-models don't have a notion of Z-rotation that would look any good
    // TODO: why do we rotate up/down then left/right?
    // rotation[1] is the up-down angle
    glRotated(-rotation[1], 1.0f, 0.0f, 0.0f);
    // rotation[0] is the left-right angle
    glRotated(rotation[0], 0.0f, 1.0f, 0.0f);
  }

  NateMash_Draw(mash);

  glPopMatrix();
}
