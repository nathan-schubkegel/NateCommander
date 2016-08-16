#include "NateMashDrawing.h"

#include <Windows.h>
#include <GL\gl.h>
#include <GL\glu.h>
#include "BoxGraphics.h"
#include "FatalErrorHandler.h"

void MyDrawNode(NateMashNode * node)
{
  size_t i, j, k, i2;
  size_t count;
  size_t stride;
  size_t vertexInputIndex;
  size_t numInputs;
  int vertIndexes[3];
  float vertParts[3];
  //size_t dataCoordinatesStride;
  NateMashSource * source;
  float * sourceData;
  int * dataIndexes;
  NateMashGeometry * geometry;

  geometry = node->geometry;

  // adjust opengl transform by the amount required for this node
  glMultMatrixf(node->transform.elements);

  glBegin( GL_TRIANGLES );

  // find the "VERTEX" input
  // (I guess we just assume every mash has a single "VERTEX" input)
  // and count the data coordinate stride
  //dataCoordinatesStride = 0;
  vertexInputIndex = 0xFFFFFFFF;
  for (i = 0; i < geometry->polylist.numInputs; i++)
  {
    //dataCoordinatesStride += mash->inputs[i].source->stride;
    if (geometry->polylist.inputs[i].dataType == NateMash_DataType_Vertex)
    {
      NateCheck(vertexInputIndex == 0xFFFFFFFF, "there can be only one!");
      vertexInputIndex = i;
    }
  }
  NateCheck(vertexInputIndex != 0xFFFFFFFF, "there must be at least one!");
  
  // get source
  // (I guess we just assume the source is using 3 floats to make a vertex)
  source = geometry->polylist.inputs[vertexInputIndex].source;
  sourceData = source->data;
  NateAssert(source->stride == 3, "we assume sources depict triangles");

  // iterate through data coordinates
  // (I guess we just assume every 3 of these is a triangle that needs to be drawn)
  //NateAssert(mash->numDataCoordinates == , "we assume numDataCoordinates depicts triangles");
  count = geometry->polylist.numDataCoordinates;
  stride = 3 * geometry->polylist.numInputs;
  dataIndexes = geometry->polylist.dataIndexes;
  numInputs = geometry->polylist.numInputs;
  NateAssert(count * stride == geometry->polylist.numDataIndexes, "right?");
  for (i = 0; i < count; i++)
  {
    i2 = i * stride + vertexInputIndex; // get to index of first vertex data index for this triangle

    // random-ish color
    glColor4ubv(colors[i % 8]);
    
    // j = for each of the 3 vertices in a triangle
    for (j = 0; j < 3; j++)
    {
      vertIndexes[j] = dataIndexes[i2 + j * numInputs];

      // k = for each of the 3 x,y,z parts of a vertex
      for (k = 0; k < 3; k++)
      {
        vertParts[k] = sourceData[vertIndexes[j] * 3 + k];
      }

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
