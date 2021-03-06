/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#ifndef NATE_COMMANDER_NATE_MASH_H
#define NATE_COMMANDER_NATE_MASH_H

#include <stdlib.h>

#define NateMash_DataType_Vertex 1
#define NateMash_DataType_Normal 2

#define NateMash_EffectType_Phong 1

typedef struct NateMashColor
{
  float rgba[4];

} NateMashColor;

typedef struct NateMashEffectPhong
{
  NateMashColor emission; // amount of light emitted from the surface of this object
  NateMashColor ambient; // amount of ambient light emitted from the surface of this object
  NateMashColor diffuse; // the basic underlying color of the material
  NateMashColor specular; //  for highlights (small bright spots on a shiny surface) by shader
  float shininess; // the specularity or roughness of the specular reflection
  float indexOfRefraction; // for perfectly refracted light as a single scalar index

} NateMashEffectPhong;

typedef struct NateMashEffect
{
  char * id;
  int effectType;

  union
  {
    NateMashEffectPhong phong;
  } data;

} NateMashEffect;

typedef struct NateMashMaterial
{
  char * id;
  NateMashEffect * effect;

} NateMashMaterial;

typedef struct NateMashSource
{
  char * id;
  size_t count; // example: 8
  size_t stride; // example: 3
  size_t totalLength; // example: 24
  float * data; // example: float[24]

} NateMashSource;

typedef struct NateMashPolyListInput
{
  int dataType;
  NateMashSource * source;

} NateMashPolyListInput;

typedef struct NateMashPolyList
{
  // <input semantic="VERTEX" source="#Cube-mesh-vertices" offset="0"/>
  // <input semantic="NORMAL" source="#Cube-mesh-normals" offset="1"/>
  NateMashPolyListInput * inputs;
  size_t numInputs;

  // <p>1 0 2 0 3 0 4 1 7 1 6 1 4 2 5 2 1 2 1 3 5 3 6 3 2 4 6 4 7 4 4 5 0 5 3 5 0 6 1 6 3 6 5 7 4 7 6 7 0 8 4 8 1 8 2 9 1 9 6 9 3 10 2 10 7 10 7 11 4 11 3 11</p>
  // every value represents the index to use from a source
  // every *numInputs* of those map to each source in *sources*
  // every 3 of those represents a single coordinate
  // example: numInputs=2
  // [ coord0_x_input0, coord0_x_input1, 
  //   coord0_y_input0, coord0_y_input1,
  //   coord0_z_input0, coord0_z_input1,
  //   coord1_x_input0, coord1_x_input1, 
  //   coord1_y_input0, coord1_y_input1,
  //   coord1_z_input0, coord1_z_input1,
  //   etc... ]
  // and every 3 of those represents a single coordinate
  int * dataIndexes;
  size_t numDataIndexes; // length of 'dataIndexes'
  size_t numDataCoordinates; // numDataIndexes / (numInputs * 3)

  char * materialSymbol; // may be null. It's a named slot for a NateMashNode to specify which
                         // material to associate with this PolyList data.

} NateMashPolyList;

typedef struct NateMashGeometry
{
  char * id;

  // could hold vertex data
  // could hold normal data
  NateMashSource * sources;
  size_t numSources;

  NateMashPolyList polylist;

} NateMashGeometry;

typedef struct NateMashMatrix
{
  float elements[16]; // in column-major order, like opengl needs

} NateMashMatrix;

typedef struct NateMashNode NateMashNode; // forward declaration

typedef struct NateMashNodeChildren
{
  NateMashNode * nodes;
  size_t numNodes;

} NateMashNodeChildren;

struct NateMashNode
{
  char * name;
  char * id; // TODO: what's the difference between these?

  NateMashMatrix transform;
  NateMashGeometry * geometry;
  char * geometryUrl;

  NateMashMaterial * material; // may be null, it's the material to use when rendering that geometry
  // FUTURE: bound materials (plural)

  NateMashNodeChildren nodes;

};

typedef struct NateMash
{
  NateMashGeometry * geometries;
  size_t numGeometries;

  NateMashEffect * effects;
  size_t numEffects;

  NateMashMaterial * materials;
  size_t numMaterials;

  NateMashNodeChildren nodes;

  // the single giant hunk of memory allocated for this NateMash
  void * zdata;

} NateMash;

// These methods malloc/free a NateMash and its internals
NateMash * NateMash_Create();
void NateMash_Destroy(NateMash * obj);

// These methods do not malloc/free the NateMash, but they malloc/free its internals
// (necessary when memory for a NateMash is allocated by Lua)
void NateMash_Init(NateMash * obj);
void NateMash_Uninit(NateMash * obj);

void NateMash_LoadFromColladaData(NateMash * obj, char * colladaFileData, size_t colladaFileLength, const char * colladaFileDebugIdentifier);
void NateMash_LoadFromColladaResourceFile(NateMash * obj, const char * mashFileName);

#endif