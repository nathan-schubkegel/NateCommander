#include "NateMesh.h"

//#include "ResourceLoader.h"
#include <string.h>
#include "ccan/NateXml/NateXml.h"
#include "ccan/NateList/NateList.h"
#include "FatalErrorHandler.h"

NateMesh * NateMesh_Create()
{
  NateMesh * obj = malloc(sizeof(NateMesh));
  if (obj == 0) return 0;
  NateMesh_Init(obj);
  return obj;
}

void NateMesh_Destroy(NateMesh * obj)
{
  NateMesh_Uninit(obj);
  free(obj);
}

void NateMesh_Init(NateMesh * obj)
{
  memset(obj, 0, sizeof(NateMesh));
}

void NateMesh_Uninit(NateMesh * obj)
{
  size_t i;

  if (obj->sources != 0)
  {
    for (i = 0; i < obj->numSources; i++)
    {
      free(obj->sources[i].data);
    }
    free(obj->sources);
  }
  obj->sources = 0;
  obj->numSources = 0;

  if (obj->inputs != 0)
  {
    free(obj->inputs);
  }
  obj->inputs = 0;
  obj->numInputs = 0;

  if (obj->dataIndexes != 0)
  {
    free(obj->dataIndexes);
  }
  obj->dataIndexes = 0;
  obj->numDataIndexes = 0;
  obj->numDataCoordinates = 0;
}

typedef struct MyNamedSource
{
  char * name;
  NateMeshSource * source;
} MyNamedSource;

typedef struct MyNamedVertice
{
  char * name;
  char * sourceName;
} MyNamedVertice;

typedef struct MyNamedPolyListInput
{
  char * sourceName;
  NateMeshPolyListInput * input;
} MyNamedPolyListInput;

typedef struct NateMeshLoadInfo
{
  NateMesh * mesh;
  char * fileName;
  NateList * sources; // holds struct MyNamedSource
  NateList * vertices; // holds struct MyNamedVertice
  NateList * polyListInputs; // holds struct MyNamedPolyListInput
} NateMeshLoadInfo;

#define NateCheckXml(condition) NateCheck2(condition, "while parsing xml file", loadInfo->fileName)
void MyProcessRoot(NateMeshLoadInfo * loadInfo, NateXmlNode * node);
void MyProcessLibraryGeometries(NateMeshLoadInfo * loadInfo, NateXmlNode * node);
void MyProcessGeometry(NateMeshLoadInfo * loadInfo, NateXmlNode * node);
void MyProcessMesh(NateMeshLoadInfo * loadInfo, NateXmlNode * node);
void MyProcessSource(NateMeshLoadInfo * loadInfo, NateXmlNode * node);
void MyProcessSourceFloatArray(NateMeshLoadInfo * loadInfo, NateXmlNode * node, NateMeshSource * source);
void MyProcessSourceTechniqueCommon(NateMeshLoadInfo * loadInfo, NateXmlNode * node, NateMeshSource * source);
void MyProcessVertices(NateMeshLoadInfo * loadInfo, NateXmlNode * node);
void MyProcessVerticesInput(NateMeshLoadInfo * loadInfo, NateXmlNode * node, MyNamedVertice * namedVertice);
void MyProcessPolylist(NateMeshLoadInfo * loadInfo, NateXmlNode * node);
void MyProcessPolylistInput(NateMeshLoadInfo * loadInfo, NateXmlNode * node);
void MyProcessPolylistVcount(NateMeshLoadInfo * loadInfo, NateXmlNode * node);
void MyProcessPolylistP(NateMeshLoadInfo * loadInfo, NateXmlNode * node);
int MyFindVertice(void * userData, void * item);
int MyFindSource(void * userData, void * item);

void MyLoadFromColladaFileCallback(
  NateXmlNode * fakeRoot,
  void * userData)
{
  NateMeshLoadInfo * loadInfo;
  size_t i;
  NateXmlNode * child;
  
  loadInfo = (NateMeshLoadInfo*)userData;
  for (i = 0; i < NateXmlNode_GetCount(fakeRoot); i++)
  {
    child = NateXmlNode_GetChild(fakeRoot, i);
    NateCheckXml(strcmp(child->ElementName, "COLLADA") == 0);
    MyProcessRoot(loadInfo, child);
  }
}

void MyProcessRoot(NateMeshLoadInfo * loadInfo, NateXmlNode * node)
{
  size_t i;
  NateXmlNode * child;

  for (i = 0; i < NateXmlNode_GetCount(node); i++)
  {
    child = NateXmlNode_GetChild(node, i);
    if (strcmp(child->ElementName, "library_geometries") == 0)
    {
      MyProcessLibraryGeometries(loadInfo, child);
    }
    // ignore other elements for now
  }
}

void MyProcessLibraryGeometries(NateMeshLoadInfo * loadInfo, NateXmlNode * node)
{
  size_t i;
  NateXmlNode * child;

  for (i = 0; i < NateXmlNode_GetCount(node); i++)
  {
    child = NateXmlNode_GetChild(node, i);
    // only 1 <geometry> is supported for now
    NateCheckXml(i == 0);
    NateCheckXml(strcmp(child->ElementName, "geometry") == 0);
    MyProcessGeometry(loadInfo, child);
  }
}

void MyProcessGeometry(NateMeshLoadInfo * loadInfo, NateXmlNode * node)
{
  size_t i;
  NateXmlNode * child;

  for (i = 0; i < NateXmlNode_GetCount(node); i++)
  {
    child = NateXmlNode_GetChild(node, i);
    // only 1 <mesh> is supported for now
    NateCheckXml(i == 0);
    NateCheckXml(strcmp(child->ElementName, "mesh") == 0);
    MyProcessMesh(loadInfo, child);
  }
}

void MyProcessMesh(NateMeshLoadInfo * loadInfo, NateXmlNode * node)
{
  size_t i;
  NateXmlNode * child;
  int numPolylists;

  numPolylists = 0;
  for (i = 0; i < NateXmlNode_GetCount(node); i++)
  {
    child = NateXmlNode_GetChild(node, i);
    if (strcmp(child->ElementName, "source") == 0)
    {
      MyProcessSource(loadInfo, child);
    }
    else if (strcmp(child->ElementName, "vertices") == 0)
    {
      MyProcessVertices(loadInfo, child);
    }
    else if (strcmp(child->ElementName, "polylist") == 0)
    {
      // only 1 polylist is supported for now
      numPolylists++;
      NateCheckXml(numPolylists == 1);
      MyProcessPolylist(loadInfo, child);
    }
  }
}

void MyProcessSource(NateMeshLoadInfo * loadInfo, NateXmlNode * node)
{
  size_t i;
  NateXmlNode * child;
  char * names[4];
  char * values[4];
  MyNamedSource * namedSource;
  NateMeshSource * source;
  int numFloatArrays;
  int numTechniqueCommons;

  // <source id="Cube-mesh-normals">
  NateCheckXml(node->AttributeCount >= 1);
  NateXml_GetAttributes(node->Attributes, 1, names, values);
  // save id attribute, it's used later
  NateCheckXml(strcmp(names[0], "id") == 0);
  // (make sure it's not in use already)
  NateCheckXml(!NateList_FindData(loadInfo->sources, MyFindSource, values[0], 0));
  // (allocate a MyNamedSource to store this name)
  namedSource = (MyNamedSource*)NateList_AddZeroedData(loadInfo->sources);
  NateCheckXml(namedSource != 0);
  namedSource->name = values[0];
  // allocate NateMeshSource while we're at it
  namedSource->source = malloc(sizeof(NateMeshSource));
  NateCheckXml(namedSource->source != 0);
  memset(namedSource->source, 0, sizeof(NateMeshSource));
  source = namedSource->source;

  numFloatArrays = 0;
  numTechniqueCommons = 0;
  for (i = 0; i < NateXmlNode_GetCount(node); i++)
  {
    child = NateXmlNode_GetChild(node, i);
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
}

void MyProcessSourceFloatArray(NateMeshLoadInfo * loadInfo, NateXmlNode * node, NateMeshSource * source)
{
  char * names[4];
  char * values[4];
  size_t i, count;
  char * next;
  char * oldNext;

  // <float_array id="Cube-mesh-normals-array" count="36">0 0 -1 </float_array>
  NateCheckXml(node->AttributeCount >= 2);
  NateXml_GetAttributes(node->Attributes, 2, names, values);
  // only one <float_array> is currently allowed per source
  NateCheckXml(source->data == 0);
  // skip "id" value, looks like it's only meaningful inside the <source>
  // and only when there are multiple <float_array> (which we currently don't support)
  NateCheckXml(strcmp(names[0], "id") == 0);
  // save 'count' attribute
  NateCheckXml(strcmp(names[1], "count") == 0);
  count = strtoul(values[1], 0, 10);
  NateCheckXml(count > 0);
  // allocate space for that many floats
  source->totalLength = count;
  source->data = malloc(count * sizeof(float));
  NateCheckXml(source->data != 0);
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

void MyProcessSourceTechniqueCommon(NateMeshLoadInfo * loadInfo, NateXmlNode * node, NateMeshSource * source)
{
  char * names[4];
  char * values[4];
  NateXmlNode * accessor;

  // <technique_common>
  //   <accessor source="#Cube-mesh-normals-array" count="12" stride="3">
  NateCheckXml(NateXmlNode_GetCount(node) == 1); // only one accessor is currently allowed per source
  accessor = NateXmlNode_GetChild(node, 0);
  NateCheckXml(strcmp(accessor->ElementName, "accessor") == 0);
  NateCheckXml(accessor->AttributeCount >= 3);
  NateCheckXml(source->count == 0); // only one technique_common and accessor is currently supported per source
  NateCheckXml(source->stride == 0);
  NateCheckXml(accessor->AttributeCount >= 3);
  NateXml_GetAttributes(accessor->Attributes, 3, names, values);
  // skip 'source' attribute and value (it just refers to the single <float_array>)
  NateCheckXml(strcmp(names[0], "source") == 0);
  // store 'count' value
  NateCheckXml(strcmp(names[1], "count") == 0);
  source->count = strtoul(values[1], 0, 10);
  NateCheckXml(source->count > 0);
  // store 'stride' value
  NateCheckXml(strcmp(names[2], "stride") == 0);
  source->stride = strtoul(values[2], 0, 10);
  NateCheckXml(source->stride > 0);
  NateCheckXml(source->stride * source->count == source->totalLength);
}

void MyProcessVertices(NateMeshLoadInfo * loadInfo, NateXmlNode * node)
{
  size_t i;
  NateXmlNode * child;
  char * names[4];
  char * values[4];
  MyNamedVertice * namedVertice;
  int numInputs;

  // <vertices id="Cube-mesh-vertices">
  NateCheckXml(node->AttributeCount >= 1);
  NateXml_GetAttributes(node->Attributes, 1, names, values);
  // save id attribute, it's used later
  NateCheckXml(strcmp(names[0], "id") == 0);
  // (make sure it's not in use already)
  NateCheckXml(!NateList_FindData(loadInfo->vertices, MyFindVertice, values[0], 0));
  // (allocate a MyNamedVertice to store the name)
  namedVertice = (MyNamedVertice*)NateList_AddZeroedData(loadInfo->vertices);
  NateCheckXml(namedVertice != 0);
  namedVertice->name = values[0];

  numInputs = 0;
  for (i = 0; i < NateXmlNode_GetCount(node); i++)
  {
    child = NateXmlNode_GetChild(node, i);
    if (strcmp(child->ElementName, "input") == 0)
    {
      numInputs++;
      NateCheckXml(numInputs == 1); // only 1 input per <vertices> is supported for now
      MyProcessVerticesInput(loadInfo, child, namedVertice);
    }
  }
  NateCheckXml(numInputs == 1); // exactly 1 is required
}

void MyProcessVerticesInput(NateMeshLoadInfo * loadInfo, NateXmlNode * node, MyNamedVertice * namedVertice)
{
  char * names[2];
  char * values[2];

  // <input semantic="POSITION" source="#Cube-mesh-positions"/>
  NateCheckXml(node->AttributeCount >= 2);
  NateXml_GetAttributes(node->Attributes, 2, names, values);
  // verify 'semantic' attribute is POSITION (it's arbitrary - that's just what I see blender exporting)
  NateCheckXml(strcmp(names[0], "semantic") == 0);
  NateCheckXml(strcmp(values[0], "POSITION") == 0);
  // save 'source' attribute, it's used later
  // it starts with # to indicate it's referencing something else (but don't save that character)
  NateCheckXml(strcmp(names[1], "source") == 0);
  NateCheckXml(values[1][0] == '#');
  // only one <input> is currently supported per single <vertices> element
  NateCheckXml(namedVertice->sourceName == 0);
  namedVertice->sourceName = values[1] + 1; // +1 is to skip the # character
}

void MyProcessPolylist(NateMeshLoadInfo * loadInfo, NateXmlNode * node)
{
  size_t i, count;
  NateXmlNode * child;
  char * names[4];
  char * values[4];
  int numVcounts;
  int numPs;

  // <polylist material="Material-material" count="12">
  NateCheckXml(node->AttributeCount >= 2);
  NateXml_GetAttributes(node->Attributes, 2, names, values);
  // skip 'material' attribute (TODO: someday consume these)
  NateCheckXml(strcmp(names[0], "material") == 0);
  // save 'count' attribute
  NateCheckXml(strcmp(names[1], "count") == 0);
  count = strtoul(values[1], 0, 10);
  NateCheckXml(count > 0);
  // only 1 polylist is currently supported
  NateCheckXml(loadInfo->mesh->numDataCoordinates == 0);
  loadInfo->mesh->numDataCoordinates = count;

  numVcounts = 0;
  numPs = 0;
  for (i = 0; i < NateXmlNode_GetCount(node); i++)
  {
    child = NateXmlNode_GetChild(node, i);
    if (strcmp(child->ElementName, "input") == 0)
    {
      MyProcessPolylistInput(loadInfo, child);
    }
    else if (strcmp(child->ElementName, "vcount") == 0)
    {
      numVcounts++;
      NateCheckXml(numVcounts == 1); // only 1 is supported for now
      MyProcessPolylistVcount(loadInfo, child);
    }
    else if (strcmp(child->ElementName, "p") == 0)
    {
      numPs++;
      NateCheckXml(numPs == 1); // only 1 is supported for now
      MyProcessPolylistP(loadInfo, child);
    }
  }
}

void MyProcessPolylistInput(NateMeshLoadInfo * loadInfo, NateXmlNode * node)
{
  size_t i;
  char * names[3];
  char * values[3];
  MyNamedPolyListInput * namedInput;

  // <input semantic="NORMAL" source="#Cube-mesh-normals" offset="1"/>
  NateCheckXml(node->AttributeCount >= 2);
  NateXml_GetAttributes(node->Attributes, (node->AttributeCount >= 3 ? 3 : 2), names, values);
  // create a MyNamedPolyListInput for this <input>
  namedInput = (MyNamedPolyListInput*)NateList_AddZeroedData(loadInfo->polyListInputs);
  NateCheckXml(namedInput != 0);
  namedInput->input = malloc(sizeof(NateMeshPolyListInput));
  NateCheckXml(namedInput->input != 0);
  memset(namedInput->input, 0, sizeof(NateMeshPolyListInput));
  // verify 'semantic' attribute is VERTEX or NORMAL (that's all we support now)
  NateCheckXml(strcmp(names[0], "semantic") == 0);
  if (strcmp(values[0], "VERTEX") == 0) namedInput->input->dataType = NateMesh_DataType_Vertex;
  else if (strcmp(values[0], "NORMAL") == 0) namedInput->input->dataType = NateMesh_DataType_Normal;
  else NateCheckXml(0 == strcmp(names[0], "Unrecognized 'semantic' attribute value"));
  // save 'source' attribute 
  // it starts with # to indicate it's referencing something else (but don't save that part)
  NateCheckXml(strcmp(names[1], "source") == 0);
  NateCheckXml(values[1][0] == '#');
  namedInput->sourceName = values[1] + 1; // + 1 to skip # character
  // get 'offset' attribute if it exists (when it doesn't exist, offset is 0)
  if (node->AttributeCount >= 3)
  {
    NateCheckXml(strcmp(names[2], "offset") == 0);
    i = strtoul(values[2], 0, 10);
  }
  else i = 0;
  // hack - assume order of <input> in XML is same as offset, so we don't have to store offset explicitly
  NateCheckXml(i == (NateList_GetCount(loadInfo->polyListInputs) - 1));
}

void MyProcessPolylistVcount(NateMeshLoadInfo * loadInfo, NateXmlNode * node)
{
  size_t i, count;
  char * next;
  char * oldNext;

  // <vcount>3 3 3 3 3 3 3 3 3 3 3 3 </vcount>
  // only one <vcount> is allowed
  // TODO: not really enforcing this since I'm not storing the vcount data
  // I see n 3's here, where n = the <polylist> 'count' attribute
  // I guess I'm assuming it's always 3s for now, and that there's exactly n of them
  next = node->ElementText;
  for (i = 0; i < loadInfo->mesh->numDataCoordinates; i++)
  {
    count = strtoul(next, &next, 10);
    NateCheckXml(count == 3);
  }
  oldNext = next;
  count = strtoul(next, &next, 10);
  NateCheckXml(count == 0);
  NateCheckXml(oldNext == next);
}

void MyProcessPolylistP(NateMeshLoadInfo * loadInfo, NateXmlNode * node)
{
  size_t i, count;
  char * next;
  char * oldNext;
  int * dataIndexes;

  // <p>1 0 2 0 3 0 4 1 7 1 6 1 4 2 5 2 1 2 1 3 5 3 6 3 2 4 6 4 7 4 4 5 0 5 3 5 0 6 1 6 3 6 5 7 4 7 6 7 0 8 4 8 1 8 2 9 1 9 6 9 3 10 2 10 7 10 7 11 4 11 3 11</p>
  // only one <p> allowed
  NateCheckXml(loadInfo->mesh->dataIndexes == 0);
  // there's going to be ([numOffsets] * 3 * count) indexes here,
  // see comments on NateMesh.dataIndexes for what it means
  count = NateList_GetCount(loadInfo->polyListInputs) * 3 * loadInfo->mesh->numDataCoordinates;
  dataIndexes = malloc(count * sizeof(int));
  NateCheckXml(dataIndexes != 0);
  loadInfo->mesh->numDataIndexes = count;
  loadInfo->mesh->dataIndexes = dataIndexes;
  next = node->ElementText;
  for (i = 0; i < count; i++)
  {
    oldNext = next;
    *dataIndexes = strtoul(next, &next, 10);
    NateCheckXml(oldNext != next);
    dataIndexes++;
  }
  oldNext = next;
  count = strtoul(next, &next, 10);
  NateCheckXml(count == 0);
  NateCheckXml(oldNext == next);
}

int MyFindVertice(void * userData, void * item)
{
  MyNamedVertice* namedVertice = (MyNamedVertice*)item;
  char * desiredName = (char*)userData;
  return (strcmp(desiredName, namedVertice->name) == 0);
}

int MyFindSource(void * userData, void * item)
{
  MyNamedSource* namedSource = (MyNamedSource*)item;
  char * desiredName = (char*)userData;
  return (strcmp(desiredName, namedSource->name) == 0);
}

void NateMesh_LoadFromColladaData(NateMesh * obj, char * colladaFileData, size_t colladaFileLength, char * colladaFileDebugIdentifier)
{
  NateMeshLoadInfo loadInfo;
  int parseResult;
  char errorBuffer[200];
  size_t count, i;
  NateMeshSource * source1;
  NateMeshSource * source2;
  NateMeshPolyListInput * input1;
  NateMeshPolyListInput * input2;
  char * sourceName;
  size_t sourceIndex;
  MyNamedPolyListInput* namedInput;

  // first free everything in 'obj'
  NateMesh_Uninit(obj);

  // init a 'NateMeshLoadInfo' and malloc its data structures for parsing assistance
  memset(&loadInfo, 0, sizeof(NateMeshLoadInfo));
  loadInfo.mesh = obj;
  if (colladaFileDebugIdentifier == 0) { colladaFileDebugIdentifier = "unspecified collada file data"; }
  loadInfo.fileName = colladaFileDebugIdentifier;
  
  loadInfo.sources = NateList_Create();
  NateCheck(loadInfo.sources != 0, "out of memory");
  NateList_SetBytesPerItem(loadInfo.sources, sizeof(MyNamedSource));

  loadInfo.vertices = NateList_Create();
  NateCheck(loadInfo.vertices != 0, "out of memory");
  NateList_SetBytesPerItem(loadInfo.vertices, sizeof(MyNamedVertice));

  loadInfo.polyListInputs = NateList_Create();
  NateCheck(loadInfo.polyListInputs != 0, "out of memory");
  NateList_SetBytesPerItem(loadInfo.polyListInputs, sizeof(MyNamedPolyListInput));

  // parse the collada file
  parseResult = NateXml_ParseDom(colladaFileData, colladaFileLength, errorBuffer, 200, &loadInfo, MyLoadFromColladaFileCallback);
  errorBuffer[199] = 0;
  NateCheck3(parseResult, "failed to parse collada mesh xml: ", colladaFileDebugIdentifier, errorBuffer);

  // copy/move sources into 'obj'
  count = NateList_GetCount(loadInfo.sources);
  obj->numSources = count;
  if (count == 0)
  {
    obj->sources = 0;
  }
  else
  {
    obj->sources = malloc(count * sizeof(NateMeshSource));
    NateCheck(obj->sources, "out of memory");
    memset(obj->sources, 0, count * sizeof(NateMeshSource));
    for (i = 0; i < count; i++)
    {
      source1 = &obj->sources[i];
      source2 = ((MyNamedSource*)NateList_GetData(loadInfo.sources, i))->source;
      memcpy(source1, source2, sizeof(NateMeshSource));
      // may as well free the dynamically allocated 'source' while we're here
      free(source2);
    }
  }

  // copy/move polyListInputs into 'obj'
  count = NateList_GetCount(loadInfo.polyListInputs);
  obj->numInputs = count;
  if (count == 0)
  {
    obj->inputs = 0;
  }
  else
  {
    obj->inputs = malloc(count * sizeof(NateMeshPolyListInput));
    NateCheck(obj->inputs, "out of memory");
    memset(obj->inputs, 0, count * sizeof(NateMeshPolyListInput));
    for (i = 0; i < count; i++)
    {
      input1 = &obj->inputs[i];
      namedInput = (MyNamedPolyListInput*)NateList_GetData(loadInfo.polyListInputs, i);
      input2 = namedInput->input;
      memcpy(input1, input2, sizeof(NateMeshPolyListInput));
      // may as well free the dynamically allocated 'input' while we're here
      free(input2);

      // look up the source in 'vertices'
      if (NateList_FindData(loadInfo.vertices, MyFindVertice, namedInput->sourceName, &sourceIndex))
      {
        sourceName = ((MyNamedVertice*)NateList_GetData(loadInfo.vertices, sourceIndex))->sourceName;
      }
      else
      {
        sourceName = namedInput->sourceName;
      }

      NateCheck(NateList_FindData(loadInfo.sources, MyFindSource, sourceName, &sourceIndex), "invalid collada data");
      input1->source = &obj->sources[sourceIndex];
    }
  }

  // clean up 'NateMeshLoadInfo' data structures
  NateList_Destroy(loadInfo.sources);
  NateList_Destroy(loadInfo.vertices);
  NateList_Destroy(loadInfo.polyListInputs);
}