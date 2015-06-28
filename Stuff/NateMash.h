#ifndef NATE_COMMANDER_NATE_MASH_H
#define NATE_COMMANDER_NATE_MASH_H

#include <stdlib.h>

#define NateMash_DataType_Vertex 1
#define NateMash_DataType_Normal 2

typedef struct NateMashSource
{
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

typedef struct NateMash
{
  // could hold vertex data
  // could hold normal data
  NateMashSource * sources;
  size_t numSources;

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

  // TODO: bones?
  //NateMash * childMeshes;
  //size_t numChildMeshes;
} NateMash;

// These methods malloc/free a NateMash and its internals
NateMash * NateMash_Create();
void NateMash_Destroy(NateMash * obj);

// These methods do not malloc/free the NateMash, but they malloc/free its internals
void NateMash_Init(NateMash * obj);
void NateMash_Uninit(NateMash * obj);

void NateMash_LoadFromColladaData(NateMash * obj, char * colladaFileData, size_t colladaFileLength, const char * colladaFileDebugIdentifier);
void NateMash_LoadFromColladaResourceFile(NateMash * obj, const char * mashFileName);

#endif