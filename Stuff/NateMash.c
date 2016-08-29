/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "NateMash.h"

//#include "ResourceLoader.h"
#include <string.h>
#include "NateXml.h"
#include "NateList.h"
#include "FatalErrorHandler.h"
#include "ResourceLoader.h"

#pragma warning( disable : 4996) // strcpy is unsafe warning

void NateMash_Init(NateMash * obj)
{
  NateCheck0(obj != 0);
  memset(obj, 0, sizeof(NateMash));
}

void NateMash_Uninit(NateMash * obj)
{
  NateCheck0(obj != 0);
  if (obj->zdata != 0)
  {
    free(obj->zdata);
  }
  memset(obj, 0, sizeof(NateMash));
}

NateMash * NateMash_Create()
{
  NateMash * obj = malloc(sizeof(NateMash));
  if (obj == 0) return 0;
  NateMash_Init(obj);
  return obj;
}

void NateMash_Destroy(NateMash * obj)
{
  NateMash_Uninit(obj);
  free(obj);
}

typedef struct NateMashLoadInfo
{
  NateMash * mash;
  const char * fileName;
  size_t spaceToAllocate;
  size_t remainingSpace;
  int isCountingRequiredSpace;
  void * nextFreeSpace;
} NateMashLoadInfo;

#define NateCheckXml(condition) NateCheck2(condition, "while parsing xml file", loadInfo->fileName)
void * MyNateMashAllocate(NateMashLoadInfo * loadInfo, size_t amount);
void MyProcessRoot(NateMashLoadInfo * loadInfo, NateXmlNode * node);
void MyProcessLibraryVisualScenes(NateMashLoadInfo * loadInfo, NateXmlNode * libraryVisualScenes);
size_t MyProcessVisualSceneNodes(NateMashLoadInfo * loadInfo, NateXmlNode * parent, int justCountChildren,
                                 NateMashNodeChildren * mashNodeContainer);
void MyProcessVisualSceneNode(NateMashLoadInfo * loadInfo, NateXmlNode * node, 
      NateXmlNode * matrix, NateXmlNode * instanceGeometry, NateMashNodeChildren * mashNodeContainer);
void MyProcessLibraryGeometries(NateMashLoadInfo * loadInfo, NateXmlNode * node);
void MyProcessGeometry(NateMashLoadInfo * loadInfo, NateXmlNode * node, NateMashGeometry * geometry);
void MyProcessMesh(NateMashLoadInfo * loadInfo, NateXmlNode * mesh, NateMashGeometry * geometry);
void MyProcessSource(NateMashLoadInfo * loadInfo, NateXmlNode * sourceNode, NateMashSource * source);
void MyProcessSourceFloatArray(NateMashLoadInfo * loadInfo, NateXmlNode * node, NateMashSource * source);
void MyProcessSourceTechniqueCommon(NateMashLoadInfo * loadInfo, NateXmlNode * node, NateMashSource * source);
void MyProcessPolylist(NateMashLoadInfo * loadInfo, NateXmlNode * polylist, 
                       NateXmlNode * meshNode, NateMashGeometry * geometry);
void MyProcessPolylistInput(NateMashLoadInfo * loadInfo, NateXmlNode * inputNode, NateXmlNode * meshNode,
                            NateMashGeometry * geometry, NateMashPolyListInput * input);
void MyProcessPolylistVcount(NateMashLoadInfo * loadInfo, NateXmlNode * node, NateMashGeometry * geometry);
void MyProcessPolylistP(NateMashLoadInfo * loadInfo, NateXmlNode * node, NateMashGeometry * geometry, int numInputs, int numDataCoordinates);
void MyProcessLibraryEffects(NateMashLoadInfo * loadInfo, NateXmlNode * node);
void MyProcessEffect(NateMashLoadInfo * loadInfo, NateXmlNode * node, NateMashEffect * effect);
void MyProcessPhong(NateMashLoadInfo * loadInfo, NateXmlNode * node, NateMashEffectPhong * phong);
void MyProcessNodeWithColorChild(NateMashLoadInfo * loadInfo, NateXmlNode * node, NateMashColor * color);
void MyProcessNodeWithFloatChild(NateMashLoadInfo * loadInfo, NateXmlNode * node, float * value);
void MyProcessLibraryMaterials(NateMashLoadInfo * loadInfo, NateXmlNode * node);
void MyProcessMaterial(NateMashLoadInfo * loadInfo, NateXmlNode * node, NateMashMaterial * material);

size_t RoundToDword(size_t amount)
{
  size_t mod = amount % 4;
  if (mod == 0) return amount;
  else return amount + (4 - mod);
}

void * MyNateMashAllocate(NateMashLoadInfo * loadInfo, size_t amount)
{
  void * meh;

  // sometimes this method is called asking for 0 bytes
  if (amount == 0)
  {
    return 0; // return a null pointer
  }

  // make sure the caller is aware of the DWORD requirement
  NateCheck(amount == RoundToDword(amount), "allocation amounts must be DWORD-sized so they're all dword aligned");
  NateCheck(amount <= loadInfo->remainingSpace, "counted and allocated amounts were mismatched");
  loadInfo->remainingSpace -= amount;
  meh = loadInfo->nextFreeSpace;
  loadInfo->nextFreeSpace = (void*)((char*)loadInfo->nextFreeSpace + amount);
  return meh;
}

void MyLoadFromColladaFileCallback(
  NateXmlNode * fakeRoot,
  void * userData)
{
  NateMashLoadInfo * loadInfo;
  NateXmlNode * collada;
  NateMash * mash;
  const char * fileName;

  loadInfo = (NateMashLoadInfo*)userData;

  // preconditions
  NateCheck(loadInfo->fileName != 0, "filename is needed for XML error message reports");
  NateCheck(loadInfo->mash != 0, "mash struct should be allocated before XML parsing takes place");
  NateCheck(loadInfo->mash->zdata == 0, "mash data should be unallocated before entering this method");

  // preserve inputs passed via 'loadInfo'
  mash = loadInfo->mash;
  fileName = loadInfo->fileName;

  // clean start
  memset(loadInfo, 0, sizeof(NateMashLoadInfo));
  memset(mash, 0, sizeof(NateMash));

  // initial state
  loadInfo->fileName = fileName;
  loadInfo->mash = mash;

  // <COLLADA xmlns="http://www.collada.org/2005/11/COLLADASchema" version="1.4.1">
  NateCheckXml(NateXmlNode_GetCount(fakeRoot) == 1); // currently can only handle one ever, due to memory allocation games
  collada = NateXmlNode_GetChild(fakeRoot, 0);
  NateCheckXml(strcmp(collada->ElementName, "COLLADA") == 0);

  // count memory needed for all data
  loadInfo->isCountingRequiredSpace = 1;
  MyProcessRoot(loadInfo, collada);

  // allocate memory needed for all data
  NateCheckXml(loadInfo->spaceToAllocate > 0);
  loadInfo->nextFreeSpace = malloc(loadInfo->spaceToAllocate);
  NateCheckXml(loadInfo->nextFreeSpace != 0);
  memset(loadInfo->nextFreeSpace, 0, loadInfo->spaceToAllocate);
  loadInfo->mash->zdata = loadInfo->nextFreeSpace;
  loadInfo->remainingSpace = loadInfo->spaceToAllocate;

  // populate data memory
  loadInfo->isCountingRequiredSpace = 0;
  MyProcessRoot(loadInfo, collada);
  NateCheckXml(loadInfo->remainingSpace == 0);
}

void MyProcessRoot(NateMashLoadInfo * loadInfo, NateXmlNode * node)
{
  int libraryGeometriesProcessed;
  int libraryEffectsProcessed;
  int libraryMaterialsProcessed;
  int libraryVisualScenesProcessed;
  size_t i;
  NateXmlNode * child;

  // effects must be processed before visual scenes and before materials
  libraryEffectsProcessed = 0;
  for (i = 0; i < NateXmlNode_GetCount(node); i++)
  {
    child = NateXmlNode_GetChild(node, i);
    if (strcmp(child->ElementName, "library_effects") == 0)
    {
      NateCheckXml(!libraryEffectsProcessed); // currently can only handle 1 ever, due to memory allocation games
      MyProcessLibraryEffects(loadInfo, child);
      libraryEffectsProcessed = 1;
    }
  }

  // materials must be processed before visual scenes and geometries
  libraryMaterialsProcessed = 0;
  for (i = 0; i < NateXmlNode_GetCount(node); i++)
  {
    child = NateXmlNode_GetChild(node, i);
    if (strcmp(child->ElementName, "library_materials") == 0)
    {
      NateCheckXml(!libraryMaterialsProcessed); // currently can only handle 1 ever, due to memory allocation games
      MyProcessLibraryMaterials(loadInfo, child);
      libraryMaterialsProcessed = 1;
    }
  }

  // geometries must be processed before visual scenes
  libraryGeometriesProcessed = 0;
  for (i = 0; i < NateXmlNode_GetCount(node); i++)
  {
    child = NateXmlNode_GetChild(node, i);
    if (strcmp(child->ElementName, "library_geometries") == 0)
    {
      NateCheckXml(!libraryGeometriesProcessed); // currently can only handle 1 ever, due to memory allocation games
      MyProcessLibraryGeometries(loadInfo, child);
      libraryGeometriesProcessed = 1;
    }
  }
  NateCheckXml(libraryGeometriesProcessed); // currently requires 1, due to memory allocation games

  // process visual scenes
  libraryVisualScenesProcessed = 0;
  for (i = 0; i < NateXmlNode_GetCount(node); i++)
  {
    child = NateXmlNode_GetChild(node, i);
    if (strcmp(child->ElementName, "library_visual_scenes") == 0)
    {
      NateCheckXml(!libraryVisualScenesProcessed); // currently can only handle 1 ever, due to memory allocation games
      MyProcessLibraryVisualScenes(loadInfo, child);
      libraryVisualScenesProcessed = 1;
    }
    // ignore other elements for now
  }
  NateCheckXml(libraryVisualScenesProcessed); // currently requires 1, due to memory allocation games
}

void MyProcessLibraryVisualScenes(NateMashLoadInfo * loadInfo, NateXmlNode * libraryVisualScenes)
{
  NateXmlNode * visualScene;
  size_t requiredChildren, spaceForChildren;
  NateMashNodeChildren * mashNodeContainer;

  // what would it mean to have multiple scenes? can't handle that yet
  NateCheckXml(NateXmlNode_GetCount(libraryVisualScenes) == 1);

  visualScene = NateXmlNode_GetChild(libraryVisualScenes, 0);
  NateCheckXml(strcmp(visualScene->ElementName, "visual_scene") == 0);

  // space required for NateMashNode children
  requiredChildren = MyProcessVisualSceneNodes(loadInfo, visualScene, 1, 0);
  spaceForChildren = RoundToDword(requiredChildren * sizeof(NateMashNode));

  // count or save all that data
  if (loadInfo->isCountingRequiredSpace)
  {
    loadInfo->spaceToAllocate += spaceForChildren;
    mashNodeContainer = 0;
  }
  else
  {
    // allocate space for NateMashNode children
    NateCheckXml(loadInfo->mash->nodes.nodes == 0); // make sure this only gets allocated once!
    loadInfo->mash->nodes.nodes = MyNateMashAllocate(loadInfo, spaceForChildren);
    // This will be incremented as children are added
    //loadInfo->mash->nodes.numNodes = requiredChildren;

    mashNodeContainer = &loadInfo->mash->nodes;
  }

  // process children
  MyProcessVisualSceneNodes(loadInfo, visualScene, 0, mashNodeContainer);
}

// returns the number of immediate child NateMashNodes that will be created
// for the given parent node
size_t MyProcessVisualSceneNodes(NateMashLoadInfo * loadInfo, NateXmlNode * parent, int justCountChildren,
                                 NateMashNodeChildren * mashNodeContainer)
{
  size_t i, j, numChildren;
  NateXmlNode * nodeNode;
  NateXmlNode * thingNode;
  NateXmlNode * matrixNode;
  NateXmlNode * instanceGeometryNode;

  numChildren = 0;

  // <visual_scene id="Scene" name="Scene">
  //   <node id="Cube_001" name="Cube_001" type="NODE">
  //     <matrix sid="transform">1 0 0 -0.4882614 0 1 0 1.015152 0 0 1 1.72046 0 0 0 1</matrix>
  //     <instance_geometry url="#Cube_001-mesh" />
  //   </node>
  // </visual_scene>
  for (i = 0; i < NateXmlNode_GetCount(parent); i++)
  {
    nodeNode = NateXmlNode_GetChild(parent, i);
    if (strcmp(nodeNode->ElementName, "node") == 0)
    {
      matrixNode = 0;
      instanceGeometryNode = 0;

      // get the 'matrix' and 'instance_geometry' child elements
      // (lights and cameras don't have 'instance_geometry' so ignore those)
      for (j = 0; j < NateXmlNode_GetCount(nodeNode); j++)
      {
        thingNode = NateXmlNode_GetChild(nodeNode, j);
        if (strcmp(thingNode->ElementName, "matrix") == 0)
        {
          // only expecting one
          NateCheckXml(matrixNode == 0);
          matrixNode = thingNode;
        }
        else if (strcmp(thingNode->ElementName, "instance_geometry") == 0)
        {
          // only expecting one
          NateCheckXml(instanceGeometryNode == 0);
          instanceGeometryNode = thingNode;
        }
      }

      if (matrixNode != 0 && instanceGeometryNode != 0)
      {
        if (!justCountChildren)
        {
          MyProcessVisualSceneNode(loadInfo, nodeNode, matrixNode, instanceGeometryNode, mashNodeContainer);
        }
        numChildren++;
      }
    }
  }

  return numChildren;
}

void MyProcessVisualSceneNode(NateMashLoadInfo * loadInfo, NateXmlNode * nodeNode, 
  NateXmlNode * matrixNode, NateXmlNode * instanceGeometryNode, NateMashNodeChildren * mashNodeContainer)
{
  char * name;
  char * id;
  char * type;
  char * matrixSid;
  char * geometryUrl;
  size_t i, end;
  char * next;
  char * oldNext;
  NateMashMatrix matrixData;
  size_t requiredChildren;
  size_t allNeededSpace;
  size_t spaceForName, spaceForId, spaceForGeometryUrl, spaceForChildren;
  NateMashNode * newMashNode;
  NateMashGeometry * geometry;

  // <node id="Baldy" name="Baldy" type="NODE">
  //   <matrix sid="transform">1 0 0 0 0 1 0 0 0 0 1 0.4905362 0 0 0 1</matrix>
  //   <instance_geometry url="#BaldySphere-mesh"/>
  // </node>

  // confirm node 'type' = "NODE", not sure what it means, but I like it
  type = NateXmlNode_GetAttribute(nodeNode, "type");
  NateCheckXml(type != 0 && strcmp(type, "NODE") == 0);

  // consume node 'name' and 'id'
  name = NateXmlNode_GetAttribute(nodeNode, "name");
  NateCheckXml(name != 0);
  id = NateXmlNode_GetAttribute(nodeNode, "id");
  NateCheckXml(id != 0);

  // confirm matrix attribute "sid" = "transform", not sure what it means, but I like it
  matrixSid = NateXmlNode_GetAttribute(matrixNode, "sid");
  NateCheckXml(matrixSid != 0 && strcmp(matrixSid, "transform") == 0);

  // consume matrix data
  next = matrixNode->ElementText;
  for (i = 0; i < 16; i++)
  {
    oldNext = next;
    // Collada matrices are written in row-major order to aid the human reader.
    // OpenGL matrices are in column-major order
    // Do some fancy shuffling to read the values correctly.
    matrixData.elements[i / 4 + 4 * (i % 4)] = (float)strtod(next, &next);
    NateCheckXml(oldNext != next);
  }
  oldNext = next;
  end = (size_t)strtod(next, &next);
  NateCheckXml(end == 0);
  NateCheckXml(oldNext == next);

  // consume instance geometry attribute 'url' = '#IdOfSomeGeometry'
  geometryUrl = NateXmlNode_GetAttribute(instanceGeometryNode, "url");
  NateCheckXml(geometryUrl != 0);
  NateCheckXml(geometryUrl[0] == '#');
  geometryUrl++;

  // count immediate children
  requiredChildren = MyProcessVisualSceneNodes(loadInfo, nodeNode, 1, 0);

  // space required for dynamic fields of NateMashNode
  allNeededSpace = 
    (spaceForName = RoundToDword(strlen(name) + 1)) +
    (spaceForId = RoundToDword(strlen(id) + 1)) + 
    (spaceForGeometryUrl = RoundToDword(strlen(geometryUrl) + 1)) +
    // space required for NateMashNode children
    (spaceForChildren = RoundToDword(requiredChildren * sizeof(NateMashNode)));

  // count or save all that data
  if (loadInfo->isCountingRequiredSpace)
  {
    loadInfo->spaceToAllocate += allNeededSpace;
    newMashNode = 0;
  }
  else
  {
    // claim space for NateMashNode object
    // (it was allocated by a parent call, but we claim it here)
    NateCheckXml(mashNodeContainer != 0); // make sure this was passed in!
    NateCheckXml(mashNodeContainer->nodes != 0); // make sure this was allocated!
    newMashNode = &mashNodeContainer->nodes[mashNodeContainer->numNodes];
    mashNodeContainer->numNodes++;

    // allocate space for NateMashNode children
    newMashNode->nodes.nodes = MyNateMashAllocate(loadInfo, spaceForChildren);
    // This will be incremented as children are added
    //newMashNode->nodes.numNodes = requiredChildren;

    // save dynamic fields of NateMashNode
    newMashNode->id = MyNateMashAllocate(loadInfo, spaceForId);
    strcpy(newMashNode->id, id);
    newMashNode->name = MyNateMashAllocate(loadInfo, spaceForName);
    strcpy(newMashNode->name, name);
    newMashNode->geometryUrl = MyNateMashAllocate(loadInfo, spaceForGeometryUrl);
    strcpy(newMashNode->geometryUrl, geometryUrl);

    // save non-dynamic fields of NateMashNode
    memcpy(&newMashNode->transform, &matrixData, sizeof(newMashNode->transform));

    // find geometry by that name, save pointer to it in NateMashNode
    NateCheckXml(newMashNode->geometry == 0);
    for (i = 0; i < loadInfo->mash->numGeometries; i++)
    {
      geometry = &loadInfo->mash->geometries[i];
      if (strcmp(geometry->id, geometryUrl) == 0)
      {
        newMashNode->geometry = geometry;
        break;
      }
    }
    NateCheckXml(newMashNode->geometry != 0);
  }

  // process children
  MyProcessVisualSceneNodes(loadInfo, nodeNode, 0, newMashNode == 0 ? 0 : &newMashNode->nodes);
}

void MyProcessLibraryGeometries(NateMashLoadInfo * loadInfo, NateXmlNode * node)
{
  size_t i, requiredChildren, spaceForChildren;
  NateXmlNode * child;
  NateMashGeometry * geometry;

  // get count of geometry children
  requiredChildren = 0;
  for (i = 0; i < NateXmlNode_GetCount(node); i++)
  {
    child = NateXmlNode_GetChild(node, i);
    NateCheckXml(strcmp(child->ElementName, "geometry") == 0);
    requiredChildren++;
  }

  // space required for referencing geometry children
  spaceForChildren = RoundToDword(requiredChildren * sizeof(NateMashGeometry));

  // count or save all that data
  if (loadInfo->isCountingRequiredSpace)
  {
    loadInfo->spaceToAllocate += spaceForChildren;
  }
  else
  {
    NateCheckXml(loadInfo->mash->geometries == 0); // make sure this only gets allocated once!
    loadInfo->mash->geometries = MyNateMashAllocate(loadInfo, spaceForChildren);
    loadInfo->mash->numGeometries = requiredChildren;
  }

  // process children
  for (i = 0; i < NateXmlNode_GetCount(node); i++)
  {
    child = NateXmlNode_GetChild(node, i);
    NateCheckXml(strcmp(child->ElementName, "geometry") == 0);

    geometry = loadInfo->isCountingRequiredSpace ? 0 : &loadInfo->mash->geometries[i];
    MyProcessGeometry(loadInfo, child, geometry);
  }
}

void MyProcessGeometry(NateMashLoadInfo * loadInfo, NateXmlNode * node, NateMashGeometry * geometry)
{
  size_t spaceForId;
  NateXmlNode * child;
  char * idValue;

  // <geometry id="Cube-mesh" name="Cube">
  // save id attribute, it's used later
  idValue = (char*)NateXmlNode_GetAttribute(node, "id");
  NateCheckXml(idValue != 0);

  spaceForId = RoundToDword(strlen(idValue) + 1);

  // count or save all that data
  if (loadInfo->isCountingRequiredSpace)
  {
    loadInfo->spaceToAllocate += spaceForId;
  }
  else
  {
    NateCheckXml(geometry->id == 0); // make sure this only gets allocated once!
    geometry->id = MyNateMashAllocate(loadInfo, spaceForId);
    strcpy(geometry->id, idValue);
  }

  // exactly 1 <mesh> per <geometry> is supported for now, 
  // until someone understands what multiple meshes means and how I'd store it in my data structures
  NateCheckXml(NateXmlNode_GetCount(node) == 1);
  child = NateXmlNode_GetChild(node, 0);
  NateCheckXml(strcmp(child->ElementName, "mesh") == 0);

  MyProcessMesh(loadInfo, child, geometry);
}

void MyProcessMesh(NateMashLoadInfo * loadInfo, NateXmlNode * mesh, NateMashGeometry * geometry)
{
  //<mesh>
  //  <source id="Rightard-mesh-positions">...</source>
  //  <source id="Rightard-mesh-normals">...</source>
  //  <vertices id="Rightard-mesh-vertices">...</vertices>
  //  <polylist count="12">...</polylist>
  //</mesh>

  size_t i, spaceForSources;
  NateXmlNode * child;
  NateXmlNode * polylist;
  int numSources, numPolylists, sourceIndex;
  NateMashSource * source;

  // count the number of child elements
  numSources = 0;
  numPolylists = 0;
  polylist = 0;
  for (i = 0; i < NateXmlNode_GetCount(mesh); i++)
  {
    child = NateXmlNode_GetChild(mesh, i);
    if (strcmp(child->ElementName, "source") == 0)
    {
      numSources++;
    }
    else if (strcmp(child->ElementName, "polylist") == 0)
    {
      numPolylists++;
      polylist = child;
    }
  }

  // count or allocate space for 'source' children
  spaceForSources = RoundToDword(numSources * sizeof(NateMashSource));
  if (loadInfo->isCountingRequiredSpace)
  {
    loadInfo->spaceToAllocate += spaceForSources;
  }
  else
  {
    NateCheckXml(geometry->sources == 0); // only allocate this once
    geometry->sources = MyNateMashAllocate(loadInfo, spaceForSources);
    geometry->numSources = numSources;
  }

  // process sources
  sourceIndex = 0;
  for (i = 0; i < NateXmlNode_GetCount(mesh); i++)
  {
    child = NateXmlNode_GetChild(mesh, i);
    if (strcmp(child->ElementName, "source") == 0)
    {
      source = 0;
      if (!loadInfo->isCountingRequiredSpace)
      {
        source = &geometry->sources[sourceIndex];
        sourceIndex++;
      }

      MyProcessSource(loadInfo, child, source);
    }
  }

  // process polylist
  NateCheckXml(numPolylists == 1); // exactly 1 polylist is supported for now
  MyProcessPolylist(loadInfo, polylist, mesh, geometry);
}

void MyProcessSource(NateMashLoadInfo * loadInfo, NateXmlNode * sourceNode, NateMashSource * source)
{
  size_t i, spaceForId;
  NateXmlNode * child;
  int numFloatArrays;
  int numTechniqueCommons;
  char * id;

  //<source id="BaldySphere-mesh-positions">
  //  <float_array id="BaldySphere-mesh-positions-array" count="126">0 0 -0.5 ... </float_array>
  //  <technique_common>
  //    <accessor source="#BaldySphere-mesh-positions-array" count="42" stride="3">
  //      <param name="X" type="float"/>
  //      <param name="Y" type="float"/>
  //      <param name="Z" type="float"/>
  //    </accessor>
  //  </technique_common>
  //</source>

  // save id attribute, it's used later
  id = NateXmlNode_GetAttribute(sourceNode, "id");
  NateCheckXml(id != 0);
  spaceForId = RoundToDword(strlen(id) + 1);
  if (loadInfo->isCountingRequiredSpace)
  {
    loadInfo->spaceToAllocate += spaceForId;
  }
  else
  {
    source->id = MyNateMashAllocate(loadInfo, spaceForId);
    strcpy(source->id, id);
  }
  
  // process child elements
  numFloatArrays = 0;
  numTechniqueCommons = 0;
  for (i = 0; i < NateXmlNode_GetCount(sourceNode); i++)
  {
    child = NateXmlNode_GetChild(sourceNode, i);
    if (strcmp(child->ElementName, "float_array") == 0)
    {
      numFloatArrays++;
      NateCheckXml(numFloatArrays == 1); // only 1 float_array is supported for now
      MyProcessSourceFloatArray(loadInfo, child, source);
    }
    else if (strcmp(child->ElementName, "technique_common") == 0)
    {
      numTechniqueCommons++;
      NateCheckXml(numTechniqueCommons == 1); // only 1 technique_common is supported for now
      MyProcessSourceTechniqueCommon(loadInfo, child, source);
    }
  }

  // these need to exist
  NateCheckXml(numFloatArrays == 1);
  NateCheckXml(numTechniqueCommons == 1);
}

void MyProcessSourceFloatArray(NateMashLoadInfo * loadInfo, NateXmlNode * node, NateMashSource * source)
{
  char * countText;
  size_t i, count, spaceForData;
  char * next;
  char * oldNext;

  //<source id="BaldySphere-mesh-positions">
  //  <float_array id="BaldySphere-mesh-positions-array" count="126">0 0 -0.5 ... </float_array>
  //  ...
  //</source>

  // capture 'count' attribute
  countText = NateXmlNode_GetAttribute(node, "count");
  NateCheckXml(countText != 0);
  count = strtoul(countText, 0, 10);
  NateCheckXml(count > 0);

  // count or allocate space for data
  spaceForData = RoundToDword(sizeof(float) * count);
  if (loadInfo->isCountingRequiredSpace)
  {
    loadInfo->spaceToAllocate += spaceForData;
  }
  else
  {
    NateCheckXml(source->data == 0); // make sure this hasn't been allocated yet
    source->data = MyNateMashAllocate(loadInfo, spaceForData);
    source->totalLength = count;
  
    // store numbers
    next = node->ElementText;
    for (i = 0; i < count; i++)
    {
      oldNext = next;
      source->data[i] = (float)strtod(next, &next);
      NateCheckXml(oldNext != next);
    }
    oldNext = next;
    count = (size_t)strtod(next, &next);
    NateCheckXml(count == 0);
    NateCheckXml(oldNext == next);
  }
}

void MyProcessSourceTechniqueCommon(NateMashLoadInfo * loadInfo, NateXmlNode * node, NateMashSource * source)
{
  NateXmlNode * accessor;
  char * countText;
  char * strideText;

  //<source id="BaldySphere-mesh-positions">
  //  ...
  //  <technique_common>
  //    <accessor source="#BaldySphere-mesh-positions-array" count="42" stride="3">
  //      <param name="X" type="float"/>
  //      <param name="Y" type="float"/>
  //      <param name="Z" type="float"/>
  //    </accessor>
  //  </technique_common>
  //</source>

  NateCheckXml(NateXmlNode_GetCount(node) == 1); // only one accessor is currently allowed per source
  accessor = NateXmlNode_GetChild(node, 0);
  NateCheckXml(strcmp(accessor->ElementName, "accessor") == 0);

  // skip 'source' attribute and value (it just refers to the single <float_array>)

  if (!loadInfo->isCountingRequiredSpace)
  {
    // only one technique_common and accessor is currently supported per source
    NateCheckXml(source->count == 0);
    NateCheckXml(source->stride == 0);

    // store 'count' attribute
    countText = NateXmlNode_GetAttribute(accessor, "count");
    NateCheckXml(countText != 0);
    source->count = strtoul(countText, 0, 10);
    NateCheckXml(source->count > 0);
    
    // store 'stride' value
    strideText = NateXmlNode_GetAttribute(accessor, "stride");
    NateCheckXml(strideText != 0);
    source->stride = strtoul(strideText, 0, 10);
    NateCheckXml(source->stride > 0);
    NateCheckXml(source->stride * source->count == source->totalLength);
  }
}

void MyProcessPolylist(NateMashLoadInfo * loadInfo, NateXmlNode * polylist, 
                       NateXmlNode * meshNode, NateMashGeometry * geometry)
{
  size_t i, spaceForInputs;
  NateXmlNode * child;
  const char * countValue;
  int numInputs;
  int numVcounts;
  int numPs;
  int numDataCoordinates;
  NateMashPolyListInput * input;
  char * materialId;
  NateMashMaterial * material;

  //<mesh>
  //  <source id="BaldySphere-mesh-positions">
  //    <float_array id="BaldySphere-mesh-positions-array" count="126">0 0 -0.5 0.3618037 ...
  //    ...
  //  </source>
  //  <source id="BaldySphere-mesh-normals">
  //    <float_array id="BaldySphere-mesh-normals-array" count="240">0.1023809 -0.3150898 ...
  //    ...
  //  </source>
  //  <vertices id="BaldySphere-mesh-vertices">
  //    <input semantic="POSITION" source="#BaldySphere-mesh-positions"/>
  //  </vertices>
  //  <polylist count="80" material="Some-Material">
  //    <input semantic="VERTEX" source="#BaldySphere-mesh-vertices" offset="0"/>
  //    <input semantic="NORMAL" source="#BaldySphere-mesh-normals" offset="1"/>
  //    <vcount>3 3 3 3 3 3 3 3 3  ...
  //    <p>0 0 13 0 12 0 1 1 13 1 15 ...
  //  </polylist>
  //</mesh>

  // save 'count' attribute as numDataCoordinates
  countValue = NateXmlNode_GetAttribute(polylist, "count");
  numDataCoordinates = strtoul(countValue, 0, 10);
  NateCheckXml(numDataCoordinates > 0);
  if (!loadInfo->isCountingRequiredSpace)
  {
    geometry->polylist.numDataCoordinates = numDataCoordinates;
  }

  // optionally a "default" material may be named
  materialId = NateXmlNode_GetAttribute(polylist, "material");
  if (materialId != 0 && !loadInfo->isCountingRequiredSpace)
  {
    // get the reference to that material
    material = 0;
    for (i = 0; i < loadInfo->mash->numMaterials; i++)
    {
      material = &loadInfo->mash->materials[i];
      if (strcmp(material->id, materialId) == 0)
      {
        break;
      }
      material = 0;
    }
    
    NateCheckXml(material != 0);
    geometry->polylist.defaultMaterial = material;
  }

  // count <input> elements
  numInputs = 0;
  for (i = 0; i < NateXmlNode_GetCount(polylist); i++)
  {
    child = NateXmlNode_GetChild(polylist, i);
    if (strcmp(child->ElementName, "input") == 0)
    {
      numInputs++;
    }
  }
  NateCheckXml(numInputs > 0); // there must be inputs, right?

  // count or allocate space for <input> children
  spaceForInputs = RoundToDword(numInputs * sizeof(NateMashPolyListInput));
  if (loadInfo->isCountingRequiredSpace)
  {
    loadInfo->spaceToAllocate += spaceForInputs;
  }
  else
  {
    NateCheckXml(geometry->polylist.inputs == 0); // only allocate this once
    geometry->polylist.inputs = MyNateMashAllocate(loadInfo, spaceForInputs);
    geometry->polylist.numInputs = numInputs;
  }

  // all <input> must be processed before <p>
  numInputs = 0;
  for (i = 0; i < NateXmlNode_GetCount(polylist); i++)
  {
    child = NateXmlNode_GetChild(polylist, i);
    if (strcmp(child->ElementName, "input") == 0)
    {
      input = 0;
      if (!loadInfo->isCountingRequiredSpace)
      {
        input = &geometry->polylist.inputs[numInputs];
      }
      MyProcessPolylistInput(loadInfo, child, meshNode, geometry, input);
      numInputs++;
    }
  }

  // exactly 1 'vcount' must be present
  // (we don't actually harvest any data from it, so this could theoretically be relaxed)
  numVcounts = 0;
  for (i = 0; i < NateXmlNode_GetCount(polylist); i++)
  {
    child = NateXmlNode_GetChild(polylist, i);
    if (strcmp(child->ElementName, "vcount") == 0)
    {
      numVcounts++;
      NateCheckXml(numVcounts == 1); // only 1 is supported for now
      MyProcessPolylistVcount(loadInfo, child, geometry);
    }
  }
  NateCheckXml(numVcounts == 1); // exactly 1 is supported for now

  // exactly 1 'p' must be present
  numPs = 0;
  for (i = 0; i < NateXmlNode_GetCount(polylist); i++)
  {
    child = NateXmlNode_GetChild(polylist, i);
    if (strcmp(child->ElementName, "p") == 0)
    {
      numPs++;
      NateCheckXml(numPs == 1); // only 1 is supported for now
      MyProcessPolylistP(loadInfo, child, geometry, numInputs, numDataCoordinates);
    }
  }
  NateCheckXml(numPs == 1); // exactly 1 is supported for now
}

void MyProcessPolylistInput(NateMashLoadInfo * loadInfo, NateXmlNode * inputNode, NateXmlNode * meshNode,
                            NateMashGeometry * geometry, NateMashPolyListInput * input)
{
  size_t i;
  char * next;
  char * semantic;
  char * source;
  char * offset;
  char * id;
  NateXmlNode * foundVertices;
  NateXmlNode * child;
  int foundSourceIndex, currentSourceIndex;

  //<mesh>
  //  <source id="Cube-mesh-positions"/>
  //  <source id="Cube-mesh-normals"/>
  //  <vertices id="Cube-mesh-vertices">
  //    <input semantic="POSITION" source="#Cube-mesh-positions"/>
  //  </vertices>
  //  <polylist material="Material-material" count="12">
  //    <input semantic="VERTEX" source="#Cube-mesh-vertices" offset="0"/>
  //    <input semantic="NORMAL" source="#Cube-mesh-normals" offset="1"/>
  //    <vcount>3 3 3 3 3 3 3 3 3 3 3 3 </vcount>
  //    <p>0 0 1 ...</p>
  //  </polylist>
  //</mesh>

  // collect attributes
  semantic = NateXmlNode_GetAttribute(inputNode, "semantic");
  NateCheckXml(semantic != 0);
  
  source = NateXmlNode_GetAttribute(inputNode, "source");
  NateCheckXml(source != 0);
  NateCheckXml(source[0] == '#');
  source++;
  
  offset = NateXmlNode_GetAttribute(inputNode, "offset");
  NateCheckXml(offset != 0);

  if (!loadInfo->isCountingRequiredSpace)
  {
    // store 'semantic' as input->dataType
    if (strcmp(semantic, "VERTEX") == 0) input->dataType = NateMash_DataType_Vertex;
    else if (strcmp(semantic, "NORMAL") == 0) input->dataType = NateMash_DataType_Normal;
    else NateCheckXml(0 == strcmp(semantic, "Unrecognized 'semantic' attribute value"));

    // look for a 'vertices' node by the id of 'source'
    foundVertices = 0;
    for (i = 0; i < NateXmlNode_GetCount(meshNode); i++)
    {
      child = NateXmlNode_GetChild(meshNode, i);
      if (strcmp(child->ElementName, "vertices") == 0)
      {
        id = NateXmlNode_GetAttribute(child, "id");
        if (id != 0)
        {
          if (strcmp(id, source) == 0)
          {
            foundVertices = child;
            break;
          }
        }
      }
    }

    if (foundVertices != 0)
    {
      // parse the 'vertices' node to get the name of the 'source' that it refers to
      for (i = 0; i < NateXmlNode_GetCount(foundVertices); i++)
      {
        child = NateXmlNode_GetChild(foundVertices, i);
        if (strcmp(child->ElementName, "input") == 0)
        {
          source = NateXmlNode_GetAttribute(child, "source");
          NateCheckXml(source != 0);
          NateCheckXml(source[0] == '#');
          source++;
          break;
        }
      }
    }

    // look for a 'source' node by the id of 'source'
    foundSourceIndex = -1;
    currentSourceIndex = -1;
    for (i = 0; i < NateXmlNode_GetCount(meshNode); i++)
    {
      child = NateXmlNode_GetChild(meshNode, i);
      if (strcmp(child->ElementName, "source") == 0)
      {
        currentSourceIndex++;
        id = NateXmlNode_GetAttribute(child, "id");
        if (id != 0)
        {
          if (strcmp(id, source) == 0)
          {
            foundSourceIndex = currentSourceIndex;
            break;
          }
        }
      }
    }

    // store 'source'
    NateCheckXml(foundSourceIndex >= 0);
    NateCheckXml(foundSourceIndex < (int)geometry->numSources);
    input->source = &geometry->sources[foundSourceIndex];

    // store 'offset'
    next = offset;
    i = strtoul(offset, &next, 10);
    NateCheckXml(next != offset);
    // hack - assume order of <input> in XML is same as offset, so we don't have to store offset explicitly
    NateCheckXml(i >= 0 && i < geometry->polylist.numInputs);
    NateCheckXml(&geometry->polylist.inputs[i] == input);
  }
}

void MyProcessPolylistVcount(NateMashLoadInfo * loadInfo, NateXmlNode * node, NateMashGeometry * geometry)
{
  size_t i, count;
  char * next;
  char * oldNext;

  if (!loadInfo->isCountingRequiredSpace)
  {
    // <vcount>3 3 3 3 3 3 3 3 3 3 3 3 </vcount>
    // only one <vcount> is allowed
    // not really enforcing this since I'm not storing the vcount data
    // I see n 3's here, where n = the <polylist> 'count' attribute
    // I guess I'm assuming it's always 3s for now, and that there's exactly n of them
    next = node->ElementText;
    NateCheckXml(geometry->polylist.numDataCoordinates > 0); // must be populated before this code executes
    for (i = 0; i < geometry->polylist.numDataCoordinates; i++)
    {
      count = strtoul(next, &next, 10);
      NateCheckXml(count == 3);
    }
    oldNext = next;
    count = strtoul(next, &next, 10);
    NateCheckXml(count == 0);
    NateCheckXml(oldNext == next);
  }
}

void MyProcessPolylistP(NateMashLoadInfo * loadInfo, NateXmlNode * node, NateMashGeometry * geometry, 
                        int numInputs, int numDataCoordinates)
{
  size_t i, count, spaceForIndexes;
  char * next;
  char * oldNext;
  int * dataIndexes;

  // <p>1 0 2 0 3 0 4 1 ... </p>
  // there's going to be ([numOffsets] * 3 * count) indexes here,
  // see comments on NateMashPolyList.dataIndexes for what it means
  count = numInputs * 3 * numDataCoordinates;
  spaceForIndexes = count * sizeof(int);

  if (loadInfo->isCountingRequiredSpace)
  {
    loadInfo->spaceToAllocate += spaceForIndexes;
  }
  else
  {
    // only one <p> allowed
    NateCheckXml(geometry->polylist.dataIndexes == 0);

    dataIndexes = MyNateMashAllocate(loadInfo, spaceForIndexes);
    geometry->polylist.numDataIndexes = count;
    geometry->polylist.dataIndexes = dataIndexes;
    next = node->ElementText;
    for (i = 0; i < count; i++)
    {
      oldNext = next;
      *dataIndexes = (int)strtoul(next, &next, 10);
      NateCheckXml(oldNext != next);
      dataIndexes++;
    }
    oldNext = next;
    count = strtoul(next, &next, 10);
    NateCheckXml(count == 0);
    NateCheckXml(oldNext == next);
  }
}

void MyProcessLibraryEffects(NateMashLoadInfo * loadInfo, NateXmlNode * node)
{
  size_t i, requiredChildren, spaceForChildren;
  NateXmlNode * child;
  NateMashEffect * effect;

  // get count of effect children
  requiredChildren = 0;
  for (i = 0; i < NateXmlNode_GetCount(node); i++)
  {
    child = NateXmlNode_GetChild(node, i);
    if (strcmp(child->ElementName, "effect") == 0)
    {
      requiredChildren++;
    }
  }

  // space required for referencing effect children
  spaceForChildren = RoundToDword(requiredChildren * sizeof(NateMashEffect));

  // count or save all that data
  if (loadInfo->isCountingRequiredSpace)
  {
    loadInfo->spaceToAllocate += spaceForChildren;
  }
  else
  {
    NateCheckXml(loadInfo->mash->effects == 0); // make sure this only gets allocated once!
    loadInfo->mash->effects = MyNateMashAllocate(loadInfo, spaceForChildren);
    loadInfo->mash->numEffects = requiredChildren;
  }

  // process children
  for (i = 0; i < NateXmlNode_GetCount(node); i++)
  {
    child = NateXmlNode_GetChild(node, i);
    if (strcmp(child->ElementName, "effect") == 0)
    {
      effect = loadInfo->isCountingRequiredSpace ? 0 : &loadInfo->mash->effects[i];
      MyProcessEffect(loadInfo, child, effect);
    }
  }
}

void MyProcessEffect(NateMashLoadInfo * loadInfo, NateXmlNode * node, NateMashEffect * effect)
{
  size_t spaceForId;
  NateXmlNode * profileNode;
  NateXmlNode * techniqueNode;
  NateXmlNode * effectNode;
  char * idValue;

  // <effect id="Material-effect">
  //   <profile_COMMON>
  //     <technique sid="common">
  //       <phong>

  // save id attribute, it's used later
  idValue = (char*)NateXmlNode_GetAttribute(node, "id");
  NateCheckXml(idValue != 0);

  // count or save id
  spaceForId = RoundToDword(strlen(idValue) + 1);
  if (loadInfo->isCountingRequiredSpace)
  {
    loadInfo->spaceToAllocate += spaceForId;
  }
  else
  {
    NateCheckXml(effect->id == 0); // make sure this only gets allocated once!
    effect->id = MyNateMashAllocate(loadInfo, spaceForId);
    strcpy(effect->id, idValue);
  }

  // only <profile_COMMON> is supported for now
  NateCheckXml(NateXmlNode_GetCount(node) == 1);
  profileNode = NateXmlNode_GetChild(node, 0);
  NateCheckXml(strcmp(profileNode->ElementName, "profile_COMMON") == 0);
  
  // only <technique> is supported for now
  NateCheckXml(NateXmlNode_GetCount(profileNode) == 1);
  techniqueNode = NateXmlNode_GetChild(profileNode, 0);
  NateCheckXml(strcmp(techniqueNode->ElementName, "technique") == 0);

  // only <phong> is supported for now
  NateCheckXml(NateXmlNode_GetCount(techniqueNode) == 1);
  effectNode = NateXmlNode_GetChild(techniqueNode, 0);
  NateCheckXml(strcmp(effectNode->ElementName, "phong") == 0);

  if (!loadInfo->isCountingRequiredSpace)
  {
    effect->effectType = NateMash_EffectType_Phong;
    MyProcessPhong(loadInfo, effectNode, &effect->data.phong);
  }
}

void MyProcessPhong(NateMashLoadInfo * loadInfo, NateXmlNode * node, NateMashEffectPhong * phong)
{
  //<phong>
  //  <emission>
  //    <color sid="emission">0 0 0 1</color>
  //  </emission>
  //  <ambient>
  //    <color sid="ambient">0 0 0 1</color>
  //  </ambient>
  //  <diffuse>
  //    <color sid="diffuse">0.64 0.64 0.64 1</color>
  //  </diffuse>
  //  <specular>
  //    <color sid="specular">0.5 0.5 0.5 1</color>
  //  </specular>
  //  <shininess>
  //    <float sid="shininess">50</float>
  //  </shininess>
  //  <index_of_refraction>
  //    <float sid="index_of_refraction">1</float>
  //  </index_of_refraction>
  //</phong>

  NateXmlNode * child;
  NateXmlNode * emissionNode;
  NateXmlNode * ambientNode;
  NateXmlNode * diffuseNode;
  NateXmlNode * specularNode;
  NateXmlNode * shininessNode;
  NateXmlNode * indexOfRefractionNode;
  size_t i;

  emissionNode = 0;
  ambientNode = 0;
  diffuseNode = 0;
  specularNode = 0;
  shininessNode = 0;
  indexOfRefractionNode = 0;

  for (i = 0; i < NateXmlNode_GetCount(node); i++)
  {
    child = NateXmlNode_GetChild(node, i);
    if (strcmp(child->ElementName, "emission") == 0)
    {
      emissionNode = child;
    }
    else if (strcmp(child->ElementName, "ambient") == 0)
    {
      ambientNode = child;
    }
    else if (strcmp(child->ElementName, "diffuse") == 0)
    {
      diffuseNode = child;
    }
    else if (strcmp(child->ElementName, "specular") == 0)
    {
      specularNode = child;
    }
    else if (strcmp(child->ElementName, "shininess") == 0)
    {
      shininessNode = child;
    }
    else if (strcmp(child->ElementName, "index_of_refraction") == 0)
    {
      indexOfRefractionNode = child;
    }
    //else
    //{
    //  NateCheckXml(strcmp(child->ElementName, "unrecognized node name") == 0);
    //}
  }

  NateCheckXml(
    emissionNode != 0 &&
    ambientNode != 0 &&
    diffuseNode != 0 &&
    specularNode != 0 &&
    shininessNode != 0 &&
    indexOfRefractionNode != 0);

  if (!loadInfo->isCountingRequiredSpace)
  {
    MyProcessNodeWithColorChild(loadInfo, emissionNode, &phong->emission);
    MyProcessNodeWithColorChild(loadInfo, ambientNode, &phong->ambient);
    MyProcessNodeWithColorChild(loadInfo, diffuseNode, &phong->diffuse);
    MyProcessNodeWithColorChild(loadInfo, specularNode, &phong->specular);

    MyProcessNodeWithFloatChild(loadInfo, shininessNode, &phong->shininess);
    MyProcessNodeWithFloatChild(loadInfo, indexOfRefractionNode, &phong->indexOfRefraction);
  }
}

void MyProcessNodeWithColorChild(NateMashLoadInfo * loadInfo, NateXmlNode * node, NateMashColor * color)
{
  NateXmlNode * colorNode;
  char * next;
  char * oldNext;
  size_t i;

  //  <diffuse>
  //    <color sid="diffuse">0.64 0.64 0.64 1</color>
  //  </diffuse>

  // exactly 1 <color> child element
  NateCheckXml(NateXmlNode_GetCount(node) == 1);
  colorNode = NateXmlNode_GetChild(node, 0);
  NateCheckXml(strcmp(colorNode->ElementName, "color") == 0);

  // store numbers
  next = colorNode->ElementText;
  for (i = 0; i < 4; i++)
  {
    oldNext = next;
    color->rgba[i] = (float)strtod(next, &next);
    NateCheckXml(oldNext != next);
  }
  oldNext = next;
  i = (size_t)strtod(next, &next);
  NateCheckXml(i == 0);
  NateCheckXml(oldNext == next);
}

void MyProcessNodeWithFloatChild(NateMashLoadInfo * loadInfo, NateXmlNode * node, float * value)
{
  NateXmlNode * floatNode;
  char * next;
  char * oldNext;
  size_t i;

  //  <shininess>
  //    <float sid="shininess">50</float>
  //  </shininess>

  // exactly 1 <float> child element
  NateCheckXml(NateXmlNode_GetCount(node) == 1);
  floatNode = NateXmlNode_GetChild(node, 0);
  NateCheckXml(strcmp(floatNode->ElementName, "float") == 0);

  // store the number
  next = floatNode->ElementText;
  oldNext = next;
  *value = (float)strtod(next, &next);
  NateCheckXml(oldNext != next);

  oldNext = next;
  i = (size_t)strtod(next, &next);
  NateCheckXml(i == 0);
  NateCheckXml(oldNext == next);
}

void MyProcessLibraryMaterials(NateMashLoadInfo * loadInfo, NateXmlNode * node)
{
  size_t i, requiredChildren, spaceForChildren;
  NateXmlNode * child;
  NateMashMaterial * material;

  // get count of material children
  requiredChildren = 0;
  for (i = 0; i < NateXmlNode_GetCount(node); i++)
  {
    child = NateXmlNode_GetChild(node, i);
    if (strcmp(child->ElementName, "material") == 0)
    {
      requiredChildren++;
    }
  }

  // space required for referencing material children
  spaceForChildren = RoundToDword(requiredChildren * sizeof(NateMashMaterial));

  // count or save all that data
  if (loadInfo->isCountingRequiredSpace)
  {
    loadInfo->spaceToAllocate += spaceForChildren;
  }
  else
  {
    NateCheckXml(loadInfo->mash->materials == 0); // make sure this only gets allocated once!
    loadInfo->mash->materials = MyNateMashAllocate(loadInfo, spaceForChildren);
    loadInfo->mash->numMaterials = requiredChildren;
  }

  // process children
  for (i = 0; i < NateXmlNode_GetCount(node); i++)
  {
    child = NateXmlNode_GetChild(node, i);
    if (strcmp(child->ElementName, "material") == 0)
    {
      material = loadInfo->isCountingRequiredSpace ? 0 : &loadInfo->mash->materials[i];
      MyProcessMaterial(loadInfo, child, material);
    }
  }
}

void MyProcessMaterial(NateMashLoadInfo * loadInfo, NateXmlNode * node, NateMashMaterial * material)
{
  // <material id="Material-material" name="Material">
  //   <instance_effect url="#Material-effect"/>
  // </material>

  size_t spaceForId;
  NateXmlNode * instanceNode;
  char * idValue;
  char * urlValue;
  size_t i;
  NateMashEffect * effect;

  // save id attribute, it's used later
  idValue = (char*)NateXmlNode_GetAttribute(node, "id");
  NateCheckXml(idValue != 0);

  // count or save id
  spaceForId = RoundToDword(strlen(idValue) + 1);
  if (loadInfo->isCountingRequiredSpace)
  {
    loadInfo->spaceToAllocate += spaceForId;
  }
  else
  {
    NateCheckXml(material->id == 0); // make sure this only gets allocated once!
    material->id = MyNateMashAllocate(loadInfo, spaceForId);
    strcpy(material->id, idValue);
  }

  // only <instance_effect> is supported for now
  NateCheckXml(NateXmlNode_GetCount(node) == 1);
  instanceNode = NateXmlNode_GetChild(node, 0);
  NateCheckXml(strcmp(instanceNode->ElementName, "instance_effect") == 0);

  // get 'url' attribute
  urlValue = NateXmlNode_GetAttribute(instanceNode, "url");
  NateCheckXml(urlValue != 0);
  NateCheckXml(urlValue[0] == '#')
  urlValue++;

  // find the effect referenced by that url
  if (!loadInfo->isCountingRequiredSpace)
  {
    effect = 0;
    for (i = 0; i < loadInfo->mash->numEffects; i++)
    {
      effect = &loadInfo->mash->effects[i];
      if (strcmp(effect->id, urlValue) == 0)
      {
        break;
      }
      effect = 0;
    }

    // save that effect reference
    NateCheckXml(effect != 0);
    material->effect = effect;
  }
}

void NateMash_LoadFromColladaData(NateMash * obj, char * colladaFileData, size_t colladaFileLength, const char * colladaFileDebugIdentifier)
{
  NateMashLoadInfo loadInfo;
  int parseResult;
  char errorBuffer[200];

  // first free everything in 'obj'
  if (obj->zdata != 0)
  {
    free(obj->zdata);
  }
  memset(obj, 0, sizeof(*obj));

  // init a 'NateMashLoadInfo' for parsing assistance
  memset(&loadInfo, 0, sizeof(NateMashLoadInfo));
  loadInfo.mash = obj;
  if (colladaFileDebugIdentifier == 0) { colladaFileDebugIdentifier = "unspecified collada file data"; }
  loadInfo.fileName = colladaFileDebugIdentifier;
  
  // parse the collada file
  parseResult = NateXml_ParseDom(colladaFileData, colladaFileLength, errorBuffer, 200, &loadInfo, MyLoadFromColladaFileCallback);
  errorBuffer[199] = 0;
  NateCheck3(parseResult, "failed to parse collada mash xml: ", colladaFileDebugIdentifier, errorBuffer);
}

void NateMash_LoadFromColladaResourceFile(NateMash * obj, const char * mashFileName)
{
  size_t fileLength;
  char * fileData;

  fileData = ResourceLoader_LoadMashFile(mashFileName, &fileLength);
  NateCheck_Sdl(fileData != 0, "failed to read collada mash file");

  NateMash_LoadFromColladaData(obj, fileData, fileLength, mashFileName);
  free(fileData);
}