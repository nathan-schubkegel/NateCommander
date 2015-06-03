#include "NateMesh.h"

//#include "ResourceLoader.h"
#include <string.h>
#include "ccan/NateXml/NateXml.h"
#include "ccan/NateTStringList/NateTStringList.h"
#include "FatalErrorHandler.h"

NateMesh * NateMesh_Create()
{
  NateMesh * obj = malloc(sizeof(NateMesh));
  memset(obj, 0, sizeof(NateMesh));
  return obj;
}

void NateMesh_Destroy(NateMesh * obj)
{
  // TODO: free dynamically allocated internals
  free(obj);
}

typedef struct NateMeshLoadInfo
{
  NateMesh * mesh;
  char * fileName;
  int step;
  NateTStringList * sources;
} NateMeshLoadInfo;

// root element must be COLLADA
// child element <library_geometries>

//<geometry id="Cube-mesh" name="Cube">
//  <mesh>
//    <source id="Cube-mesh-positions">
//      <float_array id="Cube-mesh-positions-array" count="51">1 1 -1 1 -1 -1 -1 -0.9999998 -1 -0.9999997 1 -1 1 0.9999995 1 0.9999994 -1.000001 1 -1 -0.9999997 1 -1 1 1 1.898619 -2.98023e-7 0 -0.2260947 0.473471 0.6302432 0.2260953 0.4734714 -0.6302431 0.6302432 0.4734712 0.2260949 -0.6302429 0.4734712 -0.2260951 0.3533501 1.587853 1.409261 0.353349 -0.4121473 1.409261 -1.646651 -0.4121463 1.409261 -1.64665 1.587853 1.409261</float_array>
//      <technique_common>
//        <accessor source="#Cube-mesh-positions-array" count="17" stride="3">
//          <param name="X" type="float"/>
//          <param name="Y" type="float"/>
//          <param name="Z" type="float"/>
//        </accessor>
//      </technique_common>
//    </source>
//    <source id="Cube-mesh-normals">
//      <float_array id="Cube-mesh-normals-array" count="90">0 0 -1 1.7028e-7 0.5713651 -0.820696 -4.76837e-7 -1 0 -1 2.08616e-7 -1.19209e-7 1.54146e-7 0.5746995 -0.8183646 1.19892e-7 0.5746998 0.8183644 -0.8183641 0.5747001 3.32132e-7 0.8183646 0.5746995 2.2471e-7 0 1 2.79834e-7 0.7438046 -0.6683971 0 0.7438047 -4.21175e-7 0.668397 0.7438049 0.6683969 0 0.7438047 0 -0.6683969 0 0 1 0.5347867 -3.0282e-7 0.844987 -0.5347869 0 -0.844987 -2.72448e-7 -0.5713651 0.8206959 0 0 -1 1.53252e-7 0.5713651 -0.820696 0 -1 -2.98023e-7 -1 2.38419e-7 -1.49012e-7 0.4731297 0.8522257 -0.2232928 -0.4731293 0.8522258 0.2232931 -0.2232931 0.8522261 -0.4731288 0.223293 0.8522256 0.4731297 0 1 3.63624e-7 0 0 1 0.5347867 -3.0282e-7 0.8449872 -0.5347868 0 -0.844987 -2.70444e-7 -0.5713652 0.8206959</float_array>
//      <technique_common>
//        <accessor source="#Cube-mesh-normals-array" count="30" stride="3">
//          <param name="X" type="float"/>
//          <param name="Y" type="float"/>
//          <param name="Z" type="float"/>
//        </accessor>
//      </technique_common>
//    </source>
//    <vertices id="Cube-mesh-vertices">
//      <input semantic="POSITION" source="#Cube-mesh-positions"/>
//    </vertices>
//    <polylist material="Material-material" count="30">
//      <input semantic="VERTEX" source="#Cube-mesh-vertices" offset="0"/>
//      <input semantic="NORMAL" source="#Cube-mesh-normals" offset="1"/>
//      <vcount>3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 </vcount>
//      <p>0 0 1 0 2 0 7 1 16 1 13 1 5 2 6 2 2 2 2 3 6 3 7 3 7 4 4 4 9 4 0 5 3 5 10 5 4 6 0 6 11 6 3 7 7 7 12 7 11 8 10 8 12 8 5 9 1 9 8 9 4 10 5 10 8 10 0 11 4 11 8 11 1 12 0 12 8 12 13 13 16 13 15 13 4 14 13 14 14 14 7 15 6 15 15 15 6 16 5 16 14 16 3 17 0 17 2 17 4 18 7 18 13 18 1 19 5 19 2 19 3 20 2 20 7 20 12 21 7 21 9 21 11 22 0 22 10 22 9 23 4 23 11 23 10 24 3 24 12 24 9 25 11 25 12 25 14 26 13 26 15 26 5 27 4 27 14 27 16 28 7 28 15 28 15 29 6 29 14 29</p>
//    </polylist>
//  </mesh>
//</geometry>

void MyLoadFromColladaFileCallback(
  char * elementName,
  char * attributes,
  int attributeCount,
  char * elementText,
  int depth,
  void * userData)
{
  NateMeshLoadInfo * loadInfo;
  NateMeshSource * source;
  char * next;
  size_t i;
  size_t count;
  
  (void)elementText;
  loadInfo = (NateMeshLoadInfo*)userData;
  
MyLoadFromColladaFileCallback_Reevaluate:
  switch (loadInfo->step)
  {

#define NateCheckXml(condition) NateCheck2(condition, "", loadInfo->fileName)
#define STEP_ROOT_COLLADA_NODE 0
#define STEP_WAIT_FOR_LIBRARY_GEOMETRIES 1
#define STEP_WAIT_FOR_GEMOETRY 2
#define STEP_WAIT_FOR_MESH 3
#define STEP_PROCESS_MESH_GUTS 4
#define STEP_PROCESS_SOURCE_GUTS 6
#define STEP_IGNORE_REMAINING 5

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
      // <source id="Cube-mesh-positions">
      // <vertices id="Cube-mesh-vertices">
      // <polylist material="Material-material" count="12">
      if (depth < 5)
      {
        // TODO: this means we only support loading 1 mesh right now
        loadInfo->step = STEP_IGNORE_REMAINING;
        goto MyLoadFromColladaFileCallback_Reevaluate;
      }
      NateCheckXml(depth == 5);
      if (strcmp(elementName, "source") == 0)
      {
        NateCheckXml(attributeCount >= 1);
        NateCheckXml(strcmp(attributes, "id") == 0);
        attributes += strlen(attributes) + 1;
        // store the source identifier, it's used later
        NateTStringList_SetString_Memcpy(loadInfo->sources, NateTStringList_AddToEnd, attributes);
        // allocate memory for a NateMeshSource struct and store a pointer to it in loadInfo->sources
        // (it will be our responsibility to free that memory on failure)
        source = malloc(sizeof(NateMeshSource));
        NateCheckXml(source != 0);
        memset(source, 0, sizeof(NateMeshSource));
        NateTStringList_SetObject_Ptr(loadInfo->sources, NateTStringList_LastIndex, source);
        loadInfo->step = STEP_PROCESS_SOURCE_GUTS;
        break;
      }
      else if (strcmp(elementName, "vertices") == 0)
      {
        loadInfo->step = STEP_IGNORE_REMAINING;
      }
      else if (strcmp(elementName, "polylist") == 0)
      {
        loadInfo->step = STEP_IGNORE_REMAINING;
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
        NateCheckXml(attributeCount >= 2);
        NateCheckXml(strcmp(attributes, "id") == 0);
        attributes += strlen(attributes) + 1;
        // skip id value, we assume there's just one float_array
        attributes += strlen(attributes) + 1;
        // count attribute
        NateCheckXml(strcmp(attributes, "count") == 0);
        attributes += strlen(attributes) + 1;
        // store count
        count = strtoul(attributes, 0, 10);
        NateCheckXml(count > 0);
        // get the associated NateMeshSource struct and
        // allocate space for that many floats
        source = NateTStringList_GetObject(loadInfo->sources, NateTStringList_LastIndex, 0);
        NateCheckXml(source->totalLength == 0); // only one float_array is currently allowed per source
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
      }
      else if (strcmp(elementName, "technique_common") == 0)
      {
        // nothing special to do
      }
      else if (strcmp(elementName, "accessor") == 0)
      {
        // <accessor source="#Cube-mesh-positions-array" count="8" stride="3">
        // get source struct
        source = NateTStringList_GetObject(loadInfo->sources, NateTStringList_LastIndex, 0);
        NateCheckXml(source->count == 0); // only one accessor is currently allowed per source
        NateCheckXml(source->stride == 0);
        // skip 'source' attribute and value
        NateCheckXml(attributeCount >= 3);
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

void NateMesh_LoadFromColladaData(NateMesh * obj, char * colladaFileData, size_t colladaFileLength, char * colladaFileDebugIdentifier)
{
  NateMeshLoadInfo loadInfo;
  int parseResult;
  char errorBuffer[200];
  size_t count, i;
  NateMeshSource * source1;
  NateMeshSource * source2;

  // first free everything in 'obj'
  if (obj->sources != 0)
  {
    for (i = 0; i < obj->numSources; i++)
    {
      source1 = &obj->sources[0];
      free(source1->data);
    }
    free(obj->sources);
    obj->sources = 0;
    obj->numSources = 0;
  }

  // create a 'NateMeshLoadInfo' for parsing assistance
  memset(&loadInfo, 0, sizeof(NateMeshLoadInfo));
  loadInfo.mesh = obj;
  if (colladaFileDebugIdentifier == 0) { colladaFileDebugIdentifier = "unspecified collada file data"; }
  loadInfo.fileName = colladaFileDebugIdentifier;
  loadInfo.sources = NateTStringList_Create();
  NateCheck(loadInfo.sources != 0, "out of memory");

  // parse the collada file
  parseResult = NateXml_Parse(colladaFileData, colladaFileLength, errorBuffer, 200, &loadInfo, MyLoadFromColladaFileCallback);
  errorBuffer[199] = 0;
  NateCheck3(parseResult, "failed to parse collada mesh xml: ", colladaFileDebugIdentifier, errorBuffer);

  // copy/move sources into 'obj'
  count = NateTStringList_GetCount(loadInfo.sources);
  obj->numSources = count;
  obj->sources = malloc(count * sizeof(NateMeshSource));
  NateCheck(obj->sources, "out of memory");
  memset(obj->sources, 0, count * sizeof(NateMeshSource));
  for (i = 0; i < count; i++)
  {
    source1 = &obj->sources[i];
    source2 = NateTStringList_GetObject(loadInfo.sources, i, 0);
    memcpy(source1, source2, sizeof(NateMeshSource));
    free(source2);
  }

  // final cleanup
  NateTStringList_Destroy(loadInfo.sources);
}