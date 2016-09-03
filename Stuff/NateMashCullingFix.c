/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "NateMashCullingFix.h"

#include "NateMash.h"
#include "Vectors3d.h"
#include "AngleMath.h"
#include "FatalErrorHandler.h"

void MyNateMashCullingFixNode(NateMashNode * node)
{
  size_t i, j, k, iVert, iNormal;
  size_t count;
  size_t stride;
  size_t vertexInputIndex;
  size_t normalInputIndex;
  size_t numInputs;
  int vertIndexes[3];
  int normalIndexes[3];
  NateMashSource * vertSource;
  NateMashSource * normalSource;
  float * vertData;
  float * normalData;
  int * dataIndexes;
  NateMashGeometry * geometry;

  float vertParts[3][3];
  float normalParts[3][3];
  CartesianVector3d v;
  CartesianVector3d w;
  CartesianVector3d n;
  PolarVector3d nPolar;
  PolarVector3d vPolar;
  int sameAsN;
  double xyDifference;
  double yzDifference;

  geometry = node->geometry;

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

    // j = for each of the 3 vertices in a triangle
    for (j = 0; j < 3; j++)
    {
      vertIndexes[j] = dataIndexes[iVert + j * numInputs];
      normalIndexes[j] = dataIndexes[iNormal + j * numInputs];

      // k = for each of the 3 x,y,z parts of a vertex
      for (k = 0; k < 3; k++)
      {
        vertParts[j][k] = vertData[vertIndexes[j] * 3 + k];
        normalParts[j][k] = normalData[normalIndexes[j] * 3 + k];
      }
    }

    // http://math.stackexchange.com/questions/305642/how-to-find-surface-normal-of-a-triangle
    // The cross product of two sides of a triangle equals the surface normal. 
    // So, if V = P2 - P1 and W = P3 - P1, and N is the surface normal, then:
    // Nx=(Vy*Wz)-(Vz*Wy)
    // Ny=(Vz*Wx)-(Vx*Wz)
    // Nz=(Vx*Wy)-(Vy*Wx)
    v.X = vertParts[1][0] - vertParts[0][0];
    v.Y = vertParts[1][1] - vertParts[0][1];
    v.Z = vertParts[1][2] - vertParts[0][2];
    w.X = vertParts[2][0] - vertParts[0][0];
    w.Y = vertParts[2][1] - vertParts[0][1];
    w.Z = vertParts[2][2] - vertParts[0][2];
    n.X = (v.Y * w.Z) - (v.Z * w.Y);
    n.Y = (v.Z * w.X) - (v.X * w.Z);
    n.Z = (v.X * w.Y) - (v.Y * w.X);

    // convert from cartesian [x, y, z] to spherical [r, theta, phi] coordinates
    nPolar = CartesianVector3dToPolar(n);

    // check the normals at all 3 vertices of the triangle
    // to see how many are in the same direction as the triangle's surface normal
    sameAsN = 0;
    // j = for each of the 3 vertices in a triangle
    for (j = 0; j < 3; j++)
    {
      v.X = normalParts[j][0];
      v.Y = normalParts[j][1];
      v.Z = normalParts[j][2];
      vPolar = CartesianVector3dToPolar(v);
      xyDifference = AngleMath_GetAbsAnglesBetween(vPolar.XYAngle, nPolar.XYAngle);
      yzDifference = AngleMath_GetAbsAnglesBetween(vPolar.YZAngle, vPolar.YZAngle);
      if (xyDifference < 90 && yzDifference < 90)
      {
        sameAsN++;
      }
    }
    // if at least 2 of them are in the same direction as the surface normal, then they're good
    if (sameAsN < 2)
    {
      // else, need to swap the data for 2nd and 3rd vertices so face culling occurs on the desired side
      // https://www.opengl.org/wiki/Face_Culling
      dataIndexes[iVert + 1 * numInputs] = vertIndexes[2];
      dataIndexes[iNormal + 1 * numInputs] = normalIndexes[2];
      dataIndexes[iVert + 2 * numInputs] = vertIndexes[1];
      dataIndexes[iNormal + 2 * numInputs] = normalIndexes[1];
    }
  }

  // recurse for children
  for (i = 0; i < node->nodes.numNodes; i++)
  {
    MyNateMashCullingFixNode(&node->nodes.nodes[i]);
  }
}

void NateMashCullingFix(NateMash * mash)
{
  size_t i;

  for (i = 0; i < mash->nodes.numNodes; i++)
  {
    MyNateMashCullingFixNode(&mash->nodes.nodes[i]);
  }
}