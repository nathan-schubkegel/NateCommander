#include "NateMash.h"

//#include "ResourceLoader.h"
#include <string.h>
#include "ccan/NateXml/NateXml.h"
#include "ccan/NateList/NateList.h"
#include "FatalErrorHandler.h"
#include "ResourceLoader.h"

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

void NateMash_Init(NateMash * obj)
{
  memset(obj, 0, sizeof(NateMash));
}

// This method must survive being called multiple times for a single object
void NateMash_Uninit(NateMash * obj)
{
  size_t i;
  NateMashGeometry * geometry;

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

  if (obj->geometries != 0)
  {
    for (i = 0; i < obj->numGeometries; i++)
    {
      geometry = &obj->geometries[i];

      if (geometry->inputs != 0)
      {
        free(geometry->inputs);
      }
      geometry->inputs = 0;
      geometry->numInputs = 0;

      if (geometry->dataIndexes != 0)
      {
        free(geometry->dataIndexes);
      }
      geometry->dataIndexes = 0;
      geometry->numDataIndexes = 0;
      geometry->numDataCoordinates = 0;
    }
    free(obj->geometries);
  }
  obj->geometries = 0;
  obj->numGeometries = 0;
}

typedef struct MyNamedGeometry
{
  char * name;
  size_t firstPolyListInputIndex;
  size_t polyListInputCount;
  NateMashGeometry * geometry;
} MyNamedGeometry;

typedef struct MyNamedSource
{
  char * name;
  NateMashSource * source;
} MyNamedSource;

typedef struct MyNamedVertice
{
  char * name;
  char * sourceName;
} MyNamedVertice;

typedef struct MyNamedPolyListInput
{
  char * sourceName;
  NateMashPolyListInput * input;
} MyNamedPolyListInput;

typedef struct NateMashLoadInfo
{
  NateMash * mash;
  const char * fileName;
  NateList * sources; // holds struct MyNamedSource
  NateList * vertices; // holds struct MyNamedVertice
  NateList * polyListInputs; // holds struct MyNamedPolyListInput
  NateList * geometries; // holds struct MyNamedGeometry
} NateMashLoadInfo;

#define NateCheckXml(condition) NateCheck2(condition, "while parsing xml file", loadInfo->fileName)
void MyProcessRoot(NateMashLoadInfo * loadInfo, NateXmlNode * node);
void MyProcessLibraryGeometries(NateMashLoadInfo * loadInfo, NateXmlNode * node);
void MyProcessGeometry(NateMashLoadInfo * loadInfo, NateXmlNode * node);
void MyProcessMesh(NateMashLoadInfo * loadInfo, NateXmlNode * node, MyNamedGeometry * namedGeometry);
void MyProcessSource(NateMashLoadInfo * loadInfo, NateXmlNode * node);
void MyProcessSourceFloatArray(NateMashLoadInfo * loadInfo, NateXmlNode * node, NateMashSource * source);
void MyProcessSourceTechniqueCommon(NateMashLoadInfo * loadInfo, NateXmlNode * node, NateMashSource * source);
void MyProcessVertices(NateMashLoadInfo * loadInfo, NateXmlNode * node);
void MyProcessVerticesInput(NateMashLoadInfo * loadInfo, NateXmlNode * node, MyNamedVertice * namedVertice);
void MyProcessPolylist(NateMashLoadInfo * loadInfo, NateXmlNode * node, MyNamedGeometry * namedGeometry);
void MyProcessPolylistInput(NateMashLoadInfo * loadInfo, NateXmlNode * node, MyNamedGeometry * namedGeometry);
void MyProcessPolylistVcount(NateMashLoadInfo * loadInfo, NateXmlNode * node, MyNamedGeometry * namedGeometry);
void MyProcessPolylistP(NateMashLoadInfo * loadInfo, NateXmlNode * node, MyNamedGeometry * namedGeometry);
int MyFindVertice(void * userData, void * item);
int MyFindSource(void * userData, void * item);
int MyFindGeometry(void * userData, void * item);

void MyLoadFromColladaFileCallback(
  NateXmlNode * fakeRoot,
  void * userData)
{
  NateMashLoadInfo * loadInfo;
  size_t i;
  NateXmlNode * child;
  
  loadInfo = (NateMashLoadInfo*)userData;
  for (i = 0; i < NateXmlNode_GetCount(fakeRoot); i++)
  {
    child = NateXmlNode_GetChild(fakeRoot, i);
    NateCheckXml(strcmp(child->ElementName, "COLLADA") == 0);
    MyProcessRoot(loadInfo, child);
  }
}

void MyProcessRoot(NateMashLoadInfo * loadInfo, NateXmlNode * node)
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

void MyProcessLibraryGeometries(NateMashLoadInfo * loadInfo, NateXmlNode * node)
{
  size_t i;
  NateXmlNode * child;

  for (i = 0; i < NateXmlNode_GetCount(node); i++)
  {
    child = NateXmlNode_GetChild(node, i);
    NateCheckXml(strcmp(child->ElementName, "geometry") == 0);
    MyProcessGeometry(loadInfo, child);
  }
}

void MyProcessGeometry(NateMashLoadInfo * loadInfo, NateXmlNode * node)
{
  size_t i;
  NateXmlNode * child;
  char * names[4];
  char * values[4];
  MyNamedGeometry * namedGeometry;

  // <geometry id="Cube-mesh" name="Cube">
  NateCheckXml(node->AttributeCount >= 1);
  NateXml_GetAttributes(node->Attributes, 1, names, values);
  // save id attribute, it's used later
  NateCheckXml(strcmp(names[0], "id") == 0);
  // (make sure it's not in use already)
  NateCheckXml(!NateList_FindData(loadInfo->geometries, MyFindGeometry, values[0], 0));
  // (allocate a MyNamedGeometry to store the name)
  namedGeometry = (MyNamedGeometry*)NateList_AddZeroedData(loadInfo->geometries);
  NateCheckXml(namedGeometry != 0);
  namedGeometry->name = values[0];
  // (allocate a NateMashGeometry to store other info)
  namedGeometry->geometry = malloc(sizeof(NateMashGeometry));
  NateCheckXml(namedGeometry != 0);
  memset(namedGeometry->geometry, 0, sizeof(NateMashGeometry));

  for (i = 0; i < NateXmlNode_GetCount(node); i++)
  {
    child = NateXmlNode_GetChild(node, i);
    // only 1 <mesh> per <geometry> is supported for now, until someone understands what multiple meshes means and how I'd store it in my data structures
    NateCheckXml(i == 0);
    NateCheckXml(strcmp(child->ElementName, "mesh") == 0);
    MyProcessMesh(loadInfo, child, namedGeometry);
  }
}

void MyProcessMesh(NateMashLoadInfo * loadInfo, NateXmlNode * node, MyNamedGeometry * namedGeometry)
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
      MyProcessPolylist(loadInfo, child, namedGeometry);
    }
  }
}

void MyProcessSource(NateMashLoadInfo * loadInfo, NateXmlNode * node)
{
  size_t i;
  NateXmlNode * child;
  char * names[4];
  char * values[4];
  MyNamedSource * namedSource;
  NateMashSource * source;
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
  // allocate NateMashSource while we're at it
  namedSource->source = malloc(sizeof(NateMashSource));
  NateCheckXml(namedSource->source != 0);
  memset(namedSource->source, 0, sizeof(NateMashSource));
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

void MyProcessSourceFloatArray(NateMashLoadInfo * loadInfo, NateXmlNode * node, NateMashSource * source)
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

void MyProcessSourceTechniqueCommon(NateMashLoadInfo * loadInfo, NateXmlNode * node, NateMashSource * source)
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

void MyProcessVertices(NateMashLoadInfo * loadInfo, NateXmlNode * node)
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

void MyProcessVerticesInput(NateMashLoadInfo * loadInfo, NateXmlNode * node, MyNamedVertice * namedVertice)
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

void MyProcessPolylist(NateMashLoadInfo * loadInfo, NateXmlNode * node, MyNamedGeometry * namedGeometry)
{
  size_t i, count;
  NateXmlNode * child;
  const char * countValue;
  int numInputs;
  int numVcounts;
  int numPs;

  // <polylist material="Material-material" count="12">
  // skip 'material' attribute (TODO: someday consume these)
  // save 'count' attribute
  countValue = NateXmlNode_GetAttribute(node, "count");
  count = strtoul(countValue, 0, 10);
  NateCheckXml(count > 0);
  // only 1 polylist is currently supported
  NateCheckXml(namedGeometry->geometry->numDataCoordinates == 0);
  namedGeometry->geometry->numDataCoordinates = count;

  numInputs = 0;
  numVcounts = 0;
  numPs = 0;
  for (i = 0; i < NateXmlNode_GetCount(node); i++)
  {
    child = NateXmlNode_GetChild(node, i);
    if (strcmp(child->ElementName, "input") == 0)
    {
      numInputs++;
      NateCheckXml(numPs == 0); // all <input> must be processed before <p>
      MyProcessPolylistInput(loadInfo, child, namedGeometry);
    }
    else if (strcmp(child->ElementName, "vcount") == 0)
    {
      numVcounts++;
      NateCheckXml(numVcounts == 1); // only 1 is supported for now
      MyProcessPolylistVcount(loadInfo, child, namedGeometry);
    }
    else if (strcmp(child->ElementName, "p") == 0)
    {
      numPs++;
      NateCheckXml(numInputs > 0); // <input>s must exist before <p> is processed
      NateCheckXml(numPs == 1); // only 1 is supported for now
      MyProcessPolylistP(loadInfo, child, namedGeometry);
    }
  }
}

void MyProcessPolylistInput(NateMashLoadInfo * loadInfo, NateXmlNode * node, MyNamedGeometry * namedGeometry)
{
  size_t i;
  char * names[3];
  char * values[3];
  MyNamedPolyListInput * namedInput;
  char * next;

  if (namedGeometry->polyListInputCount == 0)
  {
    namedGeometry->firstPolyListInputIndex = NateList_GetCount(loadInfo->polyListInputs);
  }
  namedGeometry->polyListInputCount++;

  // <input semantic="NORMAL" source="#Cube-mesh-normals" offset="1"/>
  NateCheckXml(node->AttributeCount >= 2);
  NateXml_GetAttributes(node->Attributes, (node->AttributeCount >= 3 ? 3 : 2), names, values);
  // create a MyNamedPolyListInput for this <input>
  namedInput = (MyNamedPolyListInput*)NateList_AddZeroedData(loadInfo->polyListInputs);
  NateCheckXml(namedInput != 0);
  namedInput->input = malloc(sizeof(NateMashPolyListInput));
  NateCheckXml(namedInput->input != 0);
  memset(namedInput->input, 0, sizeof(NateMashPolyListInput));
  // verify 'semantic' attribute is VERTEX or NORMAL (that's all we support now)
  NateCheckXml(strcmp(names[0], "semantic") == 0);
  if (strcmp(values[0], "VERTEX") == 0) namedInput->input->dataType = NateMash_DataType_Vertex;
  else if (strcmp(values[0], "NORMAL") == 0) namedInput->input->dataType = NateMash_DataType_Normal;
  else NateCheckXml(0 == strcmp(names[0], "Unrecognized 'semantic' attribute value"));
  // save 'source' attribute 
  // it starts with # to indicate it's referencing something else (but don't save that character)
  NateCheckXml(strcmp(names[1], "source") == 0);
  NateCheckXml(values[1][0] == '#');
  namedInput->sourceName = values[1] + 1; // + 1 to skip # character
  // get 'offset' attribute if it exists (when it doesn't exist, offset is 0)
  if (node->AttributeCount >= 3)
  {
    NateCheckXml(strcmp(names[2], "offset") == 0);
    next = values[2];
    i = strtoul(values[2], &next, 10);
    NateCheckXml(next != values[2]);
  }
  else i = 0;
  // hack - assume order of <input> in XML is same as offset, so we don't have to store offset explicitly
  NateCheckXml(i == namedGeometry->polyListInputCount - 1);
}

void MyProcessPolylistVcount(NateMashLoadInfo * loadInfo, NateXmlNode * node, MyNamedGeometry * namedGeometry)
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
  for (i = 0; i < namedGeometry->geometry->numDataCoordinates; i++)
  {
    count = strtoul(next, &next, 10);
    NateCheckXml(count == 3);
  }
  oldNext = next;
  count = strtoul(next, &next, 10);
  NateCheckXml(count == 0);
  NateCheckXml(oldNext == next);
}

void MyProcessPolylistP(NateMashLoadInfo * loadInfo, NateXmlNode * node, MyNamedGeometry * namedGeometry)
{
  size_t i, count;
  char * next;
  char * oldNext;
  int * dataIndexes;

  // <p>1 0 2 0 3 0 4 1 7 1 6 1 4 2 5 2 1 2 1 3 5 3 6 3 2 4 6 4 7 4 4 5 0 5 3 5 0 6 1 6 3 6 5 7 4 7 6 7 0 8 4 8 1 8 2 9 1 9 6 9 3 10 2 10 7 10 7 11 4 11 3 11</p>
  // only one <p> allowed
  NateCheckXml(namedGeometry->geometry->dataIndexes == 0);
  // there's going to be ([numOffsets] * 3 * count) indexes here,
  // see comments on NateMashGeometry.dataIndexes for what it means
  count = namedGeometry->polyListInputCount * 3 * namedGeometry->geometry->numDataCoordinates;
  dataIndexes = malloc(count * sizeof(int));
  NateCheckXml(dataIndexes != 0);
  namedGeometry->geometry->numDataIndexes = count;
  namedGeometry->geometry->dataIndexes = dataIndexes;
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

int MyFindGeometry(void * userData, void * item)
{
  MyNamedGeometry* namedGeometry = (MyNamedGeometry*)item;
  char * desiredName = (char*)userData;
  return (strcmp(desiredName, namedGeometry->name) == 0);
}

void MyCopyPolyListInputsToObj(NateMashLoadInfo * loadInfo, NateMashGeometry * destGeometry, MyNamedGeometry * namedGeometry);

void NateMash_LoadFromColladaData(NateMash * obj, char * colladaFileData, size_t colladaFileLength, const char * colladaFileDebugIdentifier)
{
  NateMashLoadInfo loadInfo;
  int parseResult;
  char errorBuffer[200];
  size_t count, i;
  NateMashSource * source1;
  NateMashSource * source2;
  NateMashGeometry * geometry1;
  NateMashGeometry * geometry2;
  MyNamedSource* namedSource;
  MyNamedPolyListInput* namedInput;
  MyNamedGeometry* namedGeometry;

  // first free everything in 'obj'
  NateMash_Uninit(obj);

  // init a 'NateMashLoadInfo' and malloc its data structures for parsing assistance
  memset(&loadInfo, 0, sizeof(NateMashLoadInfo));
  loadInfo.mash = obj;
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

  loadInfo.geometries = NateList_Create();
  NateCheck(loadInfo.geometries != 0, "out of memory");
  NateList_SetBytesPerItem(loadInfo.geometries, sizeof(MyNamedGeometry));

  // parse the collada file
  parseResult = NateXml_ParseDom(colladaFileData, colladaFileLength, errorBuffer, 200, &loadInfo, MyLoadFromColladaFileCallback);
  errorBuffer[199] = 0;
  NateCheck3(parseResult, "failed to parse collada mash xml: ", colladaFileDebugIdentifier, errorBuffer);

  // copy/move sources into 'obj'
  count = NateList_GetCount(loadInfo.sources);
  obj->numSources = count;
  if (count == 0)
  {
    obj->sources = 0;
  }
  else
  {
    obj->sources = malloc(count * sizeof(NateMashSource));
    NateCheck(obj->sources, "out of memory");
    memset(obj->sources, 0, count * sizeof(NateMashSource));
    for (i = 0; i < count; i++)
    {
      source1 = &obj->sources[i];
      source2 = ((MyNamedSource*)NateList_GetData(loadInfo.sources, i))->source;
      memcpy(source1, source2, sizeof(NateMashSource));
    }
  }

  // copy/move geometries into 'obj'
  count = NateList_GetCount(loadInfo.geometries);
  obj->numGeometries = count;
  if (count == 0)
  {
    obj->geometries = 0;
  }
  else
  {
    obj->geometries = malloc(count * sizeof(NateMashGeometry));
    NateCheck(obj->geometries, "out of memory");
    memset(obj->geometries, 0, count * sizeof(NateMashGeometry));
    for (i = 0; i < count; i++)
    {
      geometry1 = &obj->geometries[i];
      namedGeometry = (MyNamedGeometry*)NateList_GetData(loadInfo.geometries, i);
      geometry2 = namedGeometry->geometry;
      memcpy(geometry1, geometry2, sizeof(NateMashGeometry));
      
      // copy/move polyListInputs into 'destGeometry'
      MyCopyPolyListInputsToObj(&loadInfo, geometry1, namedGeometry);
    }
  }

  // clean up 'NateMashLoadInfo' data structures
  count = NateList_GetCount(loadInfo.sources);
  for (i = 0; i < count; i++)
  {
    // free the dynamically allocated 'source' objects
    namedSource = (MyNamedSource*)NateList_GetData(loadInfo.sources, i);
    free(namedSource->source);
  }
  count = NateList_GetCount(loadInfo.polyListInputs);
  for (i = 0; i < count; i++)
  {
    // free the dynamically allocated 'polylistinput' objects
    namedInput = (MyNamedPolyListInput*)NateList_GetData(loadInfo.polyListInputs, i);
    free(namedInput->input);
  }
  count = NateList_GetCount(loadInfo.polyListInputs);
  for (i = 0; i < count; i++)
  {
    // free the dynamically allocated 'geometry' objects
    namedGeometry = (MyNamedGeometry*)NateList_GetData(loadInfo.geometries, i);
    free(namedGeometry->geometry);
  }
  NateList_Destroy(loadInfo.sources);
  NateList_Destroy(loadInfo.vertices);
  NateList_Destroy(loadInfo.polyListInputs);
  NateList_Destroy(loadInfo.geometries);
}

void MyCopyPolyListInputsToObj(
  NateMashLoadInfo * loadInfo,
  NateMashGeometry * destGeometry,
  MyNamedGeometry * namedGeometry)
{
  size_t count, i;
  MyNamedPolyListInput * namedInput;
  NateMashPolyListInput * input1;
  NateMashPolyListInput * input2;
  char * sourceName;
  size_t sourceIndex;

  // copy/move polyListInputs into 'destGeometry'
  count = namedGeometry->polyListInputCount;
  destGeometry->numInputs = count;
  if (count == 0)
  {
    destGeometry->inputs = 0;
  }
  else
  {
    destGeometry->inputs = malloc(count * sizeof(NateMashPolyListInput));
    NateCheck(destGeometry->inputs, "out of memory");
    memset(destGeometry->inputs, 0, count * sizeof(NateMashPolyListInput));
    for (i = 0; i < count; i++)
    {
      input1 = &destGeometry->inputs[i];
      namedInput = (MyNamedPolyListInput*)NateList_GetData(loadInfo->polyListInputs, namedGeometry->firstPolyListInputIndex + i);
      input2 = namedInput->input;
      memcpy(input1, input2, sizeof(NateMashPolyListInput));

      // look up the source in 'vertices'
      if (NateList_FindData(loadInfo->vertices, MyFindVertice, namedInput->sourceName, &sourceIndex))
      {
        sourceName = ((MyNamedVertice*)NateList_GetData(loadInfo->vertices, sourceIndex))->sourceName;
      }
      else
      {
        sourceName = namedInput->sourceName;
      }

      NateCheck(NateList_FindData(loadInfo->sources, MyFindSource, sourceName, &sourceIndex), "invalid collada data");
      input1->source = &loadInfo->mash->sources[sourceIndex];
    }
  }
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