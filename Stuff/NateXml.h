/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#ifndef CCAN_NATE_XML_H
#define CCAN_NATE_XML_H

#include <stdlib.h>
#include "NateList.h"

typedef void (NateXml_StreamingElementCallback)(
  char * elementName,
  // attributes: name1 \0 value1 \0 name2 \0 value2 \0 ... name[n] \0 value[n] \0
  char * attributes,
  // attributeCount: value of [n] above
  size_t attributeCount,
  // elementText: body text following the element opening tag, before any children tag,
  // before any closing tag. Will be empty if element is self-closing. All other body text is discarded.
  char * elementText,
  // depth: indicates element parent/child relationships. First element has depth == 1
  size_t depth,
  // userData: opaque user pointer that was passed to NateXml_ParseStreaming
  void * userData);

// Parses XML data and fires a callback for each element.
int NateXml_ParseStreaming(
  char * data, // XML data. Will be modified by this method. Currently only supports ASCII and UTF-8.
  size_t length, // XML data length (this method doesn't assume 'data' is null terminated)
  char * errorBuffer, // if not null, parse error messages will be placed here
  size_t errorBufferLength,
  void * userData, // opaque user pointer, will be passed to 'callback'
  NateXml_StreamingElementCallback * callback); // callback, fired for every element

// you locally declare 2 arrays big enough to hold [attributeCount] pointers,
// and call this method. It will copy the attribute name/value pointers into your arrays.
void NateXml_GetAttributes(char * attributes, int attributeCount, char ** names, char ** values);

struct NateXmlNode;
typedef struct NateXmlNode NateXmlNode;
struct NateXmlNode
{
  char * ElementName;
  char * Attributes; // attributes: name1 \0 value1 \0 name2 \0 value2 \0 ... name[n] \0 value[n] \0
  size_t AttributeCount; // attributeCount: value of [n] above
  char * ElementText; // elementText: body text following the element opening tag, before any children tag,
  // before any closing tag. Will be empty if element is self-closing. All other body text is discarded.
  size_t Depth; // depth: indicates element parent/child relationships. First element has depth == 1
  void * UserData; // 0 by default, intended for application use
  NateList Children; // do not access this directly. Use the NateXmlNode* methods below
  NateXmlNode * Parent;
};

size_t NateXmlNode_GetCount(NateXmlNode * obj);
NateXmlNode * NateXmlNode_GetChild(NateXmlNode * obj, size_t index);

// attribute name lookup is case sensitive
// returns null if name not found
char * NateXmlNode_GetAttribute(NateXmlNode * obj, const char * nameToFind);

typedef void (NateXml_DomElementCallback)(
  // the "fake" root node of the document. It holds all the root nodes.
  // It has null ElementName, Attributes, ElementText, Parent, and Depth == 0.
  // (technically XML documents are only supposed to have 1 root node
  //  but I've seen plenty of hand-cobbled XML with multiple root nodes
  //  so I want to support that)
  NateXmlNode * fakeRootNode,
  // opaque user pointer that was passed to NateXml_ParseStreaming
  void * userData);

// Parses XML data and fires a callback with a DOM (Document Object Model... 
// it means "a big data structure") containing the parsed data
int NateXml_ParseDom(
  char * data, // XML data. Will be modified by this method. Currently only supports ASCII and UTF-8.
  size_t length, // XML data length (this method doesn't assume 'data' is null terminated)
  char * errorBuffer, // if not null, parse error messages will be placed here
  size_t errorBufferLength,
  void * userData, // opaque user pointer passed to 'callback'
  NateXml_DomElementCallback * callback); // callback, fired once

#endif
