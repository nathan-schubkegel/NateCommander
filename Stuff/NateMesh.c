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
  int step;
  NateList * sources; // holds struct MyNamedSource
  NateList * vertices; // holds struct MyNamedVertice
  NateList * polyListInputs; // holds struct MyNamedPolyListInput
} NateMeshLoadInfo;

void MyLoadFromColladaFileCallback(
  char * elementName,
  char * attributes,
  size_t attributeCount,
  char * elementText,
  size_t depth,
  void * userData)
{
  NateMeshLoadInfo * loadInfo;
  NateMeshSource * source;
  char * next;
  char * oldNext;
  size_t i;
  size_t count;
  int * dataIndexes;
  MyNamedSource* namedSource;
  MyNamedVertice* namedVertice;
  MyNamedPolyListInput* namedInput;
  
  (void)elementText;
  loadInfo = (NateMeshLoadInfo*)userData;
  
MyLoadFromColladaFileCallback_Reevaluate:
  switch (loadInfo->step)
  {

#define NateCheckXml(condition) NateCheck2(condition, "while parsing xml file", loadInfo->fileName)
#define STEP_ROOT_COLLADA_NODE 0
#define STEP_WAIT_FOR_LIBRARY_GEOMETRIES 1
#define STEP_WAIT_FOR_GEMOETRY 2
#define STEP_WAIT_FOR_MESH 3
#define STEP_PROCESS_MESH_GUTS 4
#define STEP_PROCESS_SOURCE_GUTS 5
#define STEP_IGNORE_REMAINING 6
#define STEP_PROCESS_VERTICES_GUTS 7
#define STEP_PROCESS_POLYLIST_GUTS 8

    case STEP_ROOT_COLLADA_NODE:
      NateCheckXml(strcmp(elementName, "COLLADA") == 0);
      loadInfo->step = STEP_WAIT_FOR_LIBRARY_GEOMETRIES;
      break;

    case STEP_WAIT_FOR_LIBRARY_GEOMETRIES:
      // <library_geometries>
      NateCheckXml(depth > 1); // no multiple root nodes
      if (depth > 2) return; // ignore children of other nodes
      if (strcmp(elementName, "library_geometries") != 0) return;
      loadInfo->step = STEP_WAIT_FOR_GEMOETRY;
      break;

    case STEP_WAIT_FOR_GEMOETRY:
      // <geometry id="Cube-mesh" name="Cube">
      NateCheckXml(depth == 3);
      NateCheckXml(strcmp(elementName, "geometry") == 0);
      // TODO: could harvest mesh name someday (it's only meaningful at the application level, not the data level)
      loadInfo->step = STEP_WAIT_FOR_MESH;
      break;

    case STEP_WAIT_FOR_MESH:
      // <mesh>
      NateCheckXml(depth == 4);
      NateCheckXml(strcmp(elementName, "mesh") == 0);
      loadInfo->step = STEP_PROCESS_MESH_GUTS;
      break;

    case STEP_PROCESS_MESH_GUTS:
      if (depth < 5)
      {
        // TODO: this means we only support loading 1 mesh right now
        loadInfo->step = STEP_IGNORE_REMAINING;
        goto MyLoadFromColladaFileCallback_Reevaluate;
      }
      NateCheckXml(depth == 5);
      // <source id="Cube-mesh-positions">
      if (strcmp(elementName, "source") == 0)
      {
        NateCheckXml(attributeCount >= 1);
        // save id attribute, it's used later
        NateCheckXml(strcmp(attributes, "id") == 0);
        attributes += strlen(attributes) + 1;
        namedSource = (MyNamedSource*)NateList_AddZeroedData(loadInfo->sources);
        NateCheckXml(namedSource != 0);
        namedSource->name = attributes;
        // allocate NateMeshSource while we're at it
        namedSource->source = malloc(sizeof(NateMeshSource));
        NateCheckXml(namedSource->source != 0);
        memset(namedSource->source, 0, sizeof(NateMeshSource));
        loadInfo->step = STEP_PROCESS_SOURCE_GUTS;
        break;
      }
      // <vertices id="Cube-mesh-vertices">
      else if (strcmp(elementName, "vertices") == 0)
      {
        NateCheckXml(attributeCount >= 1);
        // save id attribute, it's used later
        NateCheckXml(strcmp(attributes, "id") == 0);
        attributes += strlen(attributes) + 1;
        namedVertice = (MyNamedVertice*)NateList_AddZeroedData(loadInfo->vertices);
        NateCheckXml(namedVertice != 0);
        namedVertice->name = attributes;
        loadInfo->step = STEP_PROCESS_VERTICES_GUTS;
        break;
      }
      // <polylist material="Material-material" count="12">
      else if (strcmp(elementName, "polylist") == 0)
      {
        NateCheckXml(attributeCount >= 2);
        // skip 'material' attribute (TODO: someday consume these)
        NateCheckXml(strcmp(attributes, "material") == 0);
        attributes += strlen(attributes) + 1;
        attributes += strlen(attributes) + 1;
        // save 'count' attribute
        NateCheckXml(strcmp(attributes, "count") == 0);
        attributes += strlen(attributes) + 1;
        count = strtoul(attributes, 0, 10);
        NateCheckXml(count > 0);
        // only 1 polylist is currently supported
        NateCheckXml(loadInfo->mesh->numDataCoordinates == 0);
        loadInfo->mesh->numDataCoordinates = count;
        loadInfo->step = STEP_PROCESS_POLYLIST_GUTS;
        break;
      }
      else
      {
        NateCheckXml(strcmp(elementName, "Unrecognized element name") == 0);
      }
      break;

    case STEP_PROCESS_SOURCE_GUTS:
      // <float_array id="Cube-mesh-positions-array" count="3">2.607685 3.291105 -0.2332705</float_array>
      // <technique_common>
      //   <accessor source="#Cube-mesh-positions-array" count="8" stride="3">
      //     <param name="X" type="float"/>
      //     <param name="Y" type="float"/>
      //     <param name="Z" type="float"/>
      //   </accessor>
      // </technique_common
      if (depth < 6)
      {
        loadInfo->step = STEP_PROCESS_MESH_GUTS;
        goto MyLoadFromColladaFileCallback_Reevaluate;
      }
      NateCheckXml(depth >= 6);
      if (strcmp(elementName, "float_array") == 0)
      {
        // get the associated NateMeshSource struct 
        // (it was just created when we processed the parent <source> XML element)
        namedSource = (MyNamedSource*)NateList_GetData(loadInfo->sources, NateList_LastIndex);
        source = namedSource->source;
        // only one <float_array> is currently allowed per source
        NateCheckXml(source->data == 0);
        // skip "id" value, looks like it's only meaningful inside the <source>
        // and only when there are multiple <float_array> (which we currently don't support)
        NateCheckXml(attributeCount >= 2);
        NateCheckXml(strcmp(attributes, "id") == 0);
        attributes += strlen(attributes) + 1;
        attributes += strlen(attributes) + 1;
        // save 'count' attribute
        NateCheckXml(strcmp(attributes, "count") == 0);
        attributes += strlen(attributes) + 1;
        count = strtoul(attributes, 0, 10);
        NateCheckXml(count > 0);
        // allocate space for that many floats
        source->totalLength = count;
        source->data = malloc(count * sizeof(float));
        NateCheckXml(source->data != 0);
        // store numbers
        next = elementText;
        for (i = 0; i < count; i++)
        {
          NateCheckXml(next[0] != '\0');
          source->data[i] = (float)strtod(next, &next);
        }
        oldNext = next;
        count = (size_t)strtod(next, &next);
        NateCheckXml(count == 0);
        NateCheckXml(oldNext == next || next[0] == '\0');
      }
      else if (strcmp(elementName, "technique_common") == 0)
      {
        // nothing special to do
      }
      else if (strcmp(elementName, "accessor") == 0)
      {
        // <accessor source="#Cube-mesh-positions-array" count="8" stride="3">
        NateCheckXml(attributeCount >= 3);
        // get source struct
        namedSource = (MyNamedSource*)NateList_GetData(loadInfo->sources, NateList_LastIndex);
        source = namedSource->source;
        NateCheckXml(source->count == 0); // only one accessor is currently allowed per source
        NateCheckXml(source->stride == 0);
        // skip 'source' attribute and value (it just refers to the single <float_array>
        NateCheckXml(strcmp(attributes, "source") == 0);
        attributes += strlen(attributes) + 1;
        attributes += strlen(attributes) + 1;
        // store 'count' value
        NateCheckXml(strcmp(attributes, "count") == 0);
        attributes += strlen(attributes) + 1;
        source->count = strtoul(attributes, 0, 10);
        NateCheckXml(source->count > 0);
        attributes += strlen(attributes) + 1;
        // store 'stride' value
        NateCheckXml(strcmp(attributes, "stride") == 0);
        attributes += strlen(attributes) + 1;
        source->stride = strtoul(attributes, 0, 10);
        NateCheckXml(source->stride > 0);
        NateCheckXml(source->stride * source->count == source->totalLength);
      }
      else if (strcmp(elementName, "param") == 0)
      {
        // nothing special to do
      }
      else
      {
        NateCheckXml(strcmp(elementName, "Unrecognized element name") == 0);
      }
      break;

    case STEP_PROCESS_VERTICES_GUTS:
      if (depth < 6)
      {
        loadInfo->step = STEP_PROCESS_MESH_GUTS;
        goto MyLoadFromColladaFileCallback_Reevaluate;
      }
      NateCheckXml(depth == 6);
      if (strcmp(elementName, "input") == 0)
      {
        NateCheckXml(attributeCount >= 2);
        // verify 'semantic' attribute is POSITION (it's arbitrary - that's just what I see blender exporting)
        NateCheckXml(strcmp(attributes, "semantic") == 0);
        attributes += strlen(attributes) + 1;
        NateCheckXml(strcmp(attributes, "POSITION") == 0);
        attributes += strlen(attributes) + 1;
        // save 'source' attribute, it's used later
        // it starts with # to indicate it's referencing something else (but don't save that character)
        NateCheckXml(strcmp(attributes, "source") == 0);
        attributes += strlen(attributes) + 1;
        NateCheckXml(attributes[0] == '#');
        attributes++;
        // Get the struct MyNamedVertice which was created when the parent XML element was processed
        namedVertice = (MyNamedVertice*)NateList_GetData(loadInfo->vertices, NateList_LastIndex);
        // only one <input> is currently supported per single <vertices> element
        NateCheckXml(namedVertice->sourceName == 0);
        namedVertice->sourceName = attributes;
        break;
      }
      else
      {
        NateCheckXml(strcmp(elementName, "Unrecognized element name") == 0);
      }
      break;

    case STEP_PROCESS_POLYLIST_GUTS:
      if (depth < 6)
      {
        loadInfo->step = STEP_PROCESS_MESH_GUTS;
        goto MyLoadFromColladaFileCallback_Reevaluate;
      }
      NateCheckXml(depth == 6);
      if (strcmp(elementName, "input") == 0)
      {
        NateCheckXml(attributeCount >= 2);
        // create a MyNamedPolyListInput for this <input>
        namedInput = (MyNamedPolyListInput*)NateList_AddZeroedData(loadInfo->polyListInputs);
        NateCheckXml(namedInput != 0);
        namedInput->input = malloc(sizeof(NateMeshPolyListInput));
        NateCheckXml(namedInput->input != 0);
        memset(namedInput->input, 0, sizeof(NateMeshPolyListInput));
        // verify 'semantic' attribute is VERTEX or NORMAL (that's all we support now)
        NateCheckXml(strcmp(attributes, "semantic") == 0);
        attributes += strlen(attributes) + 1;
        if (strcmp(attributes, "VERTEX") == 0) namedInput->input->dataType = NateMesh_DataType_Vertex;
        else if (strcmp(attributes, "NORMAL") == 0) namedInput->input->dataType = NateMesh_DataType_Normal;
        else NateCheckXml(0 == strcmp(attributes, "Unrecognized 'semantic' attribute value"));
        attributes += strlen(attributes) + 1;
        // save 'source' attribute 
        // it starts with # to indicate it's referencing something else (but don't save that part)
        NateCheckXml(strcmp(attributes, "source") == 0);
        attributes += strlen(attributes) + 1;
        NateCheckXml(attributes[0] == '#');
        attributes++;
        namedInput->sourceName = attributes;
        attributes += strlen(attributes) + 1;
        // get 'offset' attribute if it exists (when it doesn't exist, offset is 0)
        if (attributeCount >= 3)
        {
          NateCheckXml(strcmp(attributes, "offset") == 0);
          attributes += strlen(attributes) + 1;
          i = strtoul(attributes, 0, 10);
        }
        else i = 0;
        // hack - assume order of <input> in XML is same as offset, so we don't have to store offset explicitly
        NateCheckXml(i == (NateList_GetCount(loadInfo->polyListInputs) - 1));
        break;
      }
      else if (strcmp(elementName, "vcount") == 0)
      {
        // only one <vcount> is allowed
        // TODO: not really enforcing this since I'm not storing the vcount data
        // I see n 3's here, where n = the <polylist> 'count' attribute
        // I guess I'm assuming it's always 3s for now, and that there's exactly n of them
        next = elementText;
        for (i = 0; i < loadInfo->mesh->numDataCoordinates; i++)
        {
          NateCheckXml(next[0] != '\0');
          count = strtoul(next, &next, 10);
          NateCheckXml(count == 3);
        }
        oldNext = next;
        count = strtoul(next, &next, 10);
        NateCheckXml(count == 0);
        NateCheckXml(oldNext == next || next[0] == '\0');
      }
      else if (strcmp(elementName, "p") == 0)
      {
        // only one <p> allowed
        NateCheckXml(loadInfo->mesh->dataIndexes == 0);
        // there's going to be ([numOffsets] * 3 * count) indexes here,
        // see comments on NateMesh.dataIndexes for what it means
        count = NateList_GetCount(loadInfo->polyListInputs) * 3 * loadInfo->mesh->numDataCoordinates;
        dataIndexes = malloc(count * sizeof(int));
        NateCheckXml(dataIndexes != 0);
        loadInfo->mesh->numDataIndexes = count;
        loadInfo->mesh->dataIndexes = dataIndexes;
        next = elementText;
        for (i = 0; i < count; i++)
        {
          NateCheckXml(next[0] != '\0');
          *dataIndexes = strtoul(next, &next, 10);
          dataIndexes++;
        }
        oldNext = next;
        count = strtoul(next, &next, 10);
        NateCheckXml(count == 0);
        NateCheckXml(oldNext == next || next[0] == '\0');
      }
      else
      {
        NateCheckXml(strcmp(elementName, "Unrecognized element name") == 0);
      }
      break;

    case STEP_IGNORE_REMAINING:
      break;
  }
}
/*
void NateMesh_LoadFromColladaResourceFile(NateMesh * obj, char * meshFileName)
{
  size_t fileLength;
  char * fileData;
  NateMeshLoadInfo loadInfo;
  int parseResult;
  char errorBuffer[200];

  fileData = ResourceLoader_LoadMeshFile(meshFileName, &fileLength);
  NateCheck_Sdl(fileData != 0, "failed to load collada mesh");

  memset(&loadInfo, 0, sizeof(NateMeshLoadInfo));
  loadInfo.mesh = obj;
  loadInfo.fileName = meshFileName;
  loadInfo.sources = NateTStringList_Create();
  NateCheck(loadInfo.sources != 0, "out of memory");

  parseResult = NateXml_Parse(fileData, fileLength, errorBuffer, 200, &loadInfo, MyLoadFromColladaFileCallback);
  errorBuffer[199] = 0;
  NateTStringList_Destroy(loadInfo.sources);
  NateCheck3(parseResult, "failed to parse collada mesh xml: ", meshFileName, errorBuffer);
}
*/

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
  parseResult = NateXml_ParseStreaming(colladaFileData, colladaFileLength, errorBuffer, 200, &loadInfo, MyLoadFromColladaFileCallback);
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