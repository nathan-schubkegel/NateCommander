/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "NateXml.h"

struct State;
typedef struct State State;
struct State
{
  int depth;
  char * current;
  char * end;
  int isOpening;
  int isClosing;
  char * elementName;
  int elementNameLength;
  char * attributes;
  char * attributeName;
  int attributeNameLength;
  char * attributeValue;
  int attributeValueLength;
  int attributesCount;
  char * elementText;
  int elementTextLength;
  char * extraElementText;
  int extraElementTextLength;
  char * nextData; // TODO: need a better name for this, and maybe a diagram
  char currentQuote[2];
  int lineNumber;
  int columnNumber;
  char previous;
  char * errorMessageBuffer;
  size_t errorMessageBufferLength;
};

void MyAdvance(State * state)
{
  if (state->current < state->end)
  {
    if (*(state->current) == '\n')
    {
      if (state->previous == '\r')
      {
        // no change
      }
      else
      {
        state->lineNumber++;
        state->columnNumber = 1;
      }
    }
    else if (*(state->current) == '\r')
    {
        state->lineNumber++;
        state->columnNumber = 1;
    }
    else
    {
      state->columnNumber++;
    }

    state->previous = *(state->current);
    state->current++;
  }
}

char * MyEatUntil(State * state, char * stopAtTheseChars)
{
  int stop;
  char * currentStopChar;
  char * original;

  stop = 0;
  original = state->current;

  while (state->current < state->end && !stop)
  {
    for (currentStopChar = stopAtTheseChars; *currentStopChar != 0; currentStopChar++)
    {
      if (*(state->current) == *currentStopChar)
      {
        stop = 1;
        break;
      }
    }

    if (!stop)
    {
      MyAdvance(state);
    }
  }

  return original;
}

int MyEatThroughString(State * state, char * stopAtThisString)
{
  int found;
  int numToCompare;

  found = 0;
  numToCompare = strlen(stopAtThisString);

  while (state->current < state->end)
  {
    // don't allow strncmp to read state->end
    // just eat remaining characters if there aren't enough to compare
    if ((state->end - state->current) < numToCompare)
    {
      while (state->current < state->end)
      {
        MyAdvance(state);
      }
      break;
    }

    if (strncmp(state->current, stopAtThisString, numToCompare) == 0)
    {
      found = 1;
      while (numToCompare > 0)
      {
        MyAdvance(state);
        numToCompare--;
      }
      break;
    }
    else
    {
      MyAdvance(state);
    }
  }

  return found;
}

char MyEatIfChar(State * state, char * stopAtTheseChars)
{
  char * currentStopChar;
  if (state->current < state->end)
  {
    for (currentStopChar = stopAtTheseChars; *currentStopChar != 0; currentStopChar++)
    {
      if (*(state->current) == *currentStopChar)
      {
        MyAdvance(state);
        return *currentStopChar;
      }
    }
  }

  return 0;
}

int MyEatIfString(State * state, char * stringToEat)
{
  int numToCompare;

  numToCompare = strlen(stringToEat);

  if (state->current < state->end)
  {
    // don't allow strncmp to read state->end
    // just return false if there aren't enough to compare
    if ((state->end - state->current) < numToCompare)
    {
      return 0;
    }

    if (strncmp(state->current, stringToEat, numToCompare) == 0)
    {
      while (numToCompare > 0)
      {
        MyAdvance(state);
        numToCompare--;
      }
      return 1;
    }
  }

  return 0;
}

#define MyRequireChar(ampState, requiredChar) { if (!MyRequireChar2((ampState), (requiredChar))) return 0; }

int MyRequireChar2(State * state, char requiredChar)
{
  if (state->current >= state->end)
  {
    if (state->errorMessageBuffer != 0)
    {
      sprintf_s(state->errorMessageBuffer, state->errorMessageBufferLength, 
        "Unexpected end of data at line %d character %d: expected character '%c'.", 
        state->lineNumber, state->columnNumber, requiredChar);
    }

    return 0;
  }

  if (*(state->current) != requiredChar)
  {
    if (state->errorMessageBuffer != 0)
    {
      sprintf_s(state->errorMessageBuffer, state->errorMessageBufferLength, 
        "Unexpected character '%c' at line %d character %d: expected character '%c'.", 
        *(state->current), state->lineNumber, state->columnNumber, requiredChar);
    }

    return 0;
  }

  MyAdvance(state);

  return 1;
}

void MyEatOptionalWhitespace(State * state)
{
  int stop;
  stop = 0;
  while (state->current < state->end && !stop)
  {
    switch(*(state->current))
    {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
      MyAdvance(state);
      break;
    default:
      stop = 1;
      break;
    }
  }
}

#define MyRequireNonZeroData(ampState, original, messagePart) { if (!MyRequireNonZeroData2((ampState), (original), (messagePart))) return 0; }

int MyRequireNonZeroData2(State * state, char * original, char * messagePart)
{
  if (original == state->current)
  {
    if (state->errorMessageBuffer != 0)
    {
      sprintf_s(state->errorMessageBuffer, state->errorMessageBufferLength, 
        "Unexpected data at line %d character %d: expected %s.", 
        state->lineNumber, state->columnNumber, messagePart);
    }

    return 0;
  }

  return 1;
}

#define MyError(ampState, messagePart) { MyError2((ampState), (messagePart)); return 0; }

void MyError2(State * state, char * messagePart)
{
  if (state->errorMessageBuffer != 0)
  {
    sprintf_s(state->errorMessageBuffer, state->errorMessageBufferLength, 
      "Unexpected data at line %d character %d: %s.", 
      state->lineNumber, state->columnNumber, messagePart);
  }
}

#define MyErrorIfEndOfFile(ampState) { if (!MyErrorIfEndOfFile2((ampState))) return 0; }

int MyErrorIfEndOfFile2(State * state)
{
  if (state->current >= state->end)
  {
    if (state->errorMessageBuffer != 0)
    {
      sprintf_s(state->errorMessageBuffer, state->errorMessageBufferLength, 
        "Unexpected end of content at line %d character %d.", 
        state->lineNumber, state->columnNumber);
    }
    return 0;
  }
  return 1;
}

void RemoveEscapes(char * string)
{
  unsigned long value;
  int i, j;
  int len;

  len = strlen(string);
  for (i = 0; i < len; i++)
  {
    // find &
    if (string[i] != '&')
    {
      continue;
    }

    // find matching ;
    for (j = i; j < len; j++)
    {
      if (string[j] == ';')
      {
        break;
      }
    }

    // if there's no matching ; convert it to '?'
    // and null remaining characters so they get slurped
    if (j == len)
    {
      string[i] = '?';
      for (j = i + 1; j < len; j++)
      {
        string[j] = 0;
      }
      break;
    }

    // guarantee only the &stuff; chars are inspected
    string[j] = 0;

    // decimal notation?
    if (string[i+1] == '#')
    {
      value = strtoul(&string[i+2], 0, 10);
    }
    // hex notation?
    else if (string[i+1] == 'u' || string[i+1] == 'U')
    {
      value = strtoul(&string[i+2], 0, 16);
    }
    else if (strcmp(&string[i+1], "amp") == 0)
    {
      value = '&';
    }
    else if (strcmp(&string[i+1], "apos") == 0)
    {
      value = '\'';
    }
    else if (strcmp(&string[i+1], "quot") == 0)
    {
      value = '"';
    }
    else if (strcmp(&string[i+1], "gt") == 0)
    {
      value = '>';
    }
    else if (strcmp(&string[i+1], "lt") == 0)
    {
      value = '<';
    }
    else
    {
      // unrecognized mark
      value = '?';
    }

    // I don't support UTF-8 yet
    // null characters would bork this parser
    if (value <= 0 || value > 127)
    {
      value = '?';
    }

    string[i] = (char)value;

    // zero out remaining characters, they will be shifted later
    for (i++; i <= j; i++)
    {
      string[i] = 0;
    }

    i = j;
  }

  // pack all non-zero bytes toward the front of the string
  j = 0;
  for (i = 0; i < len; i++)
  {
    if (string[i] != 0)
    {
      string[j] = string[i];
      j++;
    }
  }

  // zero out remaining bytes
  while (j < len)
  {
    string[j] = 0;
    j++;
  }
}

void PackNextData(State * state, char ** string, int length)
{
  int i;
  for (i = 0; i < length; i++)
  {
    state->nextData[i] = (*string)[i];
  }
  state->nextData[length] = 0;

  RemoveEscapes(state->nextData);

  *string = state->nextData;
  state->nextData += strlen(*string) + 1;
}

void PackAdditionalData(State * state, char * extraData, int length)
{
  int i;

  state->nextData--; // back up over previous string null terminator

  for (i = 0; i < length; i++)
  {
    state->nextData[i] = extraData[i];
  }
  state->nextData[length] = 0;

  RemoveEscapes(state->nextData);

  state->nextData += strlen(state->nextData) + 1;
}

int EatCommentsAndProcessorInstructions(State * state)
{
  if (MyEatIfString(state, "<?"))
  {
    // processing instructions are ignored by this parser
    // eat all text until ?>
    // TODO: this forgivably insufficient (what about attributes with ?> in them?)
    if (!MyEatThroughString(state, "?>")) 
    {
      MyError(state, "expected ?>");
    }
    return 1;
  }
  else if (MyEatIfString(state, "<!"))
  {
    // comments are ignored by this parser 
    MyRequireChar(state, '-');
    MyRequireChar(state, '-');

    // eat all text until -->
    // TODO: comments are that simple right?
    MyEatThroughString(state, "-->");
    return 1;
  }
  return 0;
}

int NateXml_ParseStreaming(
  char * data, 
  size_t length, 
  char * errorBuffer, 
  size_t errorBufferLength, 
  void * userData, 
  NateXml_StreamingElementCallback * callback)
{
  int i;
  State state;

  memset(&state, 0, sizeof(state));
  state.current = data;
  state.end = data + length;
  state.lineNumber = 1;
  state.columnNumber = 1;
  state.nextData = data;
  state.errorMessageBuffer = errorBuffer;
  state.errorMessageBufferLength = errorBufferLength;

  // replace all zero-bytes with "space" character
  // because I think zero-bytes will mess things up
  for (i = 0; i < (int)length; i++) 
  {
    if (data[i] == 0) 
    {
      data[i] = ' ';
    }
    // replace all non-ASCII bytes with question mark character
    // because I'm not UTF-8 compliant yet
    else if (data[i] > 127)
    {
      data[i] = '?';
    }
  }

  // TODO: handle UTF-8 and leading text more appropriately
  // (this code just skips any ByteOrderMark before the first <)
  MyEatUntil(&state, "<");
  state.lineNumber = 1;
  state.columnNumber = 1;
  // keep eating until comments/PIs/text are gone
  // (TODO: technically that stuff shouldn't exist before the first element... 
  //  so this parser is too accepting here... right?)
  while (EatCommentsAndProcessorInstructions(&state)) 
  {
    MyEatUntil(&state, "<");
  }

  // :StartOfElement:
StartOfElement:
  state.elementName = 0;
  state.attributeName = 0;
  state.attributeValue = 0;
  state.attributes = 0;
  state.attributesCount = 0;
  state.elementText = 0;
  state.extraElementText = 0;
  state.isClosing = 0;
  state.isOpening = 0;

  // <
  MyRequireChar(&state, '<');

  // what kind of element?
  i = MyEatIfChar(&state, "/?!");

  // processing instructions are ignored by this parser
  if (i == '?')
  {
    // eat all text until ?>
    // TODO: this forgivably insufficient (what about attributes with ?> in them?)
    if (!MyEatThroughString(&state, "?>")) 
    {
      MyError(&state, "expected ?>");
    }

    // throw away content until next element
    // TODO: this is more disruptive than I'd like
    MyEatUntil(&state, "<");
    
    goto AfterElementProcessed;
  }

  // comments are ignored by this parser 
  if (i == '!')
  {
    MyRequireChar(&state, '-');
    MyRequireChar(&state, '-');

    // eat all text until -->
    // TODO: comments are that simple right?
    MyEatThroughString(&state, "-->");

    // throw away content until next element
    // TODO: this is more disruptive than I'd like
    MyEatUntil(&state, "<");
    
    goto AfterElementProcessed;
  }

  if (i == '/')
  {
    state.isClosing = 1;
    state.isOpening = 0;
  }
  else // it's just some element name
  {
    state.isClosing = 0;
    state.isOpening = 1;
    state.depth++;
  }

  // sequence of characters not including whitespace, >, or /   *elementName
  state.elementName = MyEatUntil(&state, " \t\r\n>/");
  MyRequireNonZeroData(&state, state.elementName, "element name");
  MyErrorIfEndOfFile(&state);
  state.elementNameLength = state.current - state.elementName;
  PackNextData(&state, &state.elementName, state.elementNameLength);

  // :AttributesOrEndOfElementTag:
AttributesOrEndOfElementTag:

  // optional whitespace
  MyEatOptionalWhitespace(&state);

  // expecting attribute or /> or >
  i = MyEatIfChar(&state, "/>");
  if (i == '/')
  {
    // error if isClosing = true already
    if (state.isClosing) 
    {
      MyError(&state, "character '/' is invalid at both beginning and end of element tag");
    }
    state.isClosing = 1;

    // >
    MyRequireChar(&state, '>');
  }
  // else if >,
  else if (i == '>')
  {
  }
  else // attribute
  {
    // error if attributes are encountered for closing tags
    if (state.isClosing)
    {
      MyError(&state, "attributes are invalid in element closing tag");
    }

    // sequence of characters other than whitespace, =
    state.attributeName = MyEatUntil(&state, " \t\r\n=");
    MyRequireNonZeroData(&state, state.attributeName, "attribute name");
    state.attributeNameLength = state.current - state.attributeName;
    PackNextData(&state, &state.attributeName, state.attributeNameLength);

    // optional whitespace
    MyEatOptionalWhitespace(&state);

    // =
    MyRequireChar(&state, '=');

    // optional whitespace
    MyEatOptionalWhitespace(&state);

    // " or ' (record which as currentQuote)
    i = MyEatIfChar(&state, "\"'");
    if (i == '\"' || i == '\'')
    {
      state.currentQuote[0] = (char)i;
    }
    else
    {
      MyError(&state, "expected quote or apostrophe to wrap attribute value");
    }

    // sequence of characters other than currentQuote
    state.attributeValue = MyEatUntil(&state, state.currentQuote);
    state.attributeValueLength = state.current - state.attributeValue;
    PackNextData(&state, &state.attributeValue, state.attributeValueLength);

    // currentQuote
    MyRequireChar(&state, state.currentQuote[0]);

    // record first attribute (it will be passed to callback)
    if (state.attributes == 0)
    {
      state.attributes = state.attributeName;
    }
    state.attributesCount++;

    goto AttributesOrEndOfElementTag;
  }

  // optional sequence of characters other than < *textContent
  state.elementText = MyEatUntil(&state, "<");
  state.elementTextLength = state.current - state.elementText;
  PackNextData(&state, &state.elementText, state.elementTextLength);

  // keep eating until comments/PIs are gone
  while (EatCommentsAndProcessorInstructions(&state)) 
  {
    state.extraElementText = MyEatUntil(&state, "<");
    state.extraElementTextLength = state.current - state.extraElementText;
    PackAdditionalData(&state, state.extraElementText, state.extraElementTextLength);
  }

  // fire element event
  if (state.isOpening)
  {
    // only include element text if this is not a self-closing element
    if (state.isClosing)
    {
      state.elementText[0] = 0;
    }
    
    (*callback)(
      state.elementName, 
      state.attributes, 
      state.attributesCount, 
      state.elementText, 
      state.depth, 
      userData);
  }

  // depth-- (error if depth < 0)
  if (state.isClosing)
  {
    state.depth--;
    if (state.depth < 0)
    {
      MyError(&state, "Too many (unbalanced) closing XML element tag(s)");
    }
  }

AfterElementProcessed:

  // if EOF && depth == 0 then done
  if (state.current >= state.end)
  {
    if (state.depth == 0)
    {
      return 1;
    }
    else
    {
      MyError(&state, "Unexpected end of data, there are unclosed XML element(s)");
    }
  }
  else
  {
    // goto :StartOfElement:
    goto StartOfElement;
  }
}

void NateXml_GetAttributes(char * attributes, int attributeCount, char ** names, char ** values)
{
  int i;

  for (i = 0; i < attributeCount; i++)
  {
    names[i] = attributes;
    attributes += strlen(attributes) + 1;
    values[i] = attributes;
    attributes += strlen(attributes) + 1;
  }
}

typedef struct MyParseDomInfo
{
  int ok;
  NateXmlNode * fakeRoot;
  NateXmlNode * previousNode;
} MyParseDomInfo;

void MyParseDomCallback(
  char * elementName,
  char * attributes,
  size_t attributeCount,
  char * elementText,
  size_t depth,
  void * userData)
{
  NateXmlNode * newNode;
  NateXmlNode * parent;
  MyParseDomInfo * parseInfo;
  size_t i;
  
  parseInfo = (MyParseDomInfo*)userData;

  // Once an error occurs, ignore remaining data
  if (!parseInfo->ok) return;

  // Figure out the new node's parent
  if (depth == parseInfo->previousNode->Depth + 1)
  {
    // it's a child of the previous node
    parent = parseInfo->previousNode;
  }
  else if (depth == parseInfo->previousNode->Depth)
  {
    // it's a sibling of the previous node
    parent = parseInfo->previousNode->Parent;
  }
  else if (depth < parseInfo->previousNode->Depth)
  {
    // it's a child of some ancestor node. Find that ancestor
    parent = parseInfo->previousNode->Parent;
    i = (parseInfo->previousNode->Depth - depth);
    while (i > 0)
    {
      if (parent == 0) // this should never happen
      {
        break;
      }
      parent = parent->Parent;
      i--;
    }
  }
  else // depth > parseInfo->previousNode->Depth + 1
  {
    // this should never happen
    parent = 0;
  }

  if (parent == 0)
  {
    parseInfo->ok = 0;
    return;
  }

  if (!NateList_AddPtr(&parent->Children, 0))
  {
    parseInfo->ok = 0;
    return;
  }

  // Allocate memory for the new node
  newNode = malloc(sizeof(NateXmlNode));
  if (newNode == 0)
  {
    parseInfo->ok = 0;
    return;
  }
  memset(newNode, 0, sizeof(NateXmlNode));
  NateList_Init(&newNode->Children);

  newNode->ElementName = elementName;
  newNode->Attributes = attributes;
  newNode->AttributeCount = attributeCount;
  newNode->ElementText = elementText;
  newNode->Depth = depth;
  newNode->Parent = parent;

  NateList_SetPtr(&parent->Children, NateList_LastIndex, newNode);
  parseInfo->previousNode = newNode;
}

// Parses XML data and fires a callback with a DOM (Document Object Model... 
// it means "a big data structure") containing the parsed data
int NateXml_ParseDom(
  char * data, // XML data. Will be modified by this method. Currently only supports ASCII and UTF-8.
  size_t length, // XML data length (this method doesn't assume 'data' is null terminated)
  char * errorBuffer, // if not null, parse error messages will be placed here
  size_t errorBufferLength,
  void * userData, // opaque user pointer passed to 'callback'
  NateXml_DomElementCallback * callback) // callback, fired once
{
  MyParseDomInfo parseInfo;
  NateXmlNode * current;
  NateXmlNode * parent;
  size_t count;
  int parseResult;

  memset(&parseInfo, 0, sizeof(MyParseDomInfo));
  parseInfo.ok = 1;
  parseInfo.fakeRoot = malloc(sizeof(NateXmlNode));
  if (parseInfo.fakeRoot == 0)
  {
    if (errorBuffer != 0)
    {
      sprintf_s(errorBuffer, errorBufferLength, "out of memory");
    }
    return 0;
  }
  parseInfo.previousNode = parseInfo.fakeRoot;

  memset(parseInfo.fakeRoot, 0, sizeof(NateXmlNode));
  NateList_Init(&parseInfo.fakeRoot->Children);
  parseResult = NateXml_ParseStreaming(data, length, errorBuffer, errorBufferLength, &parseInfo, MyParseDomCallback);

  // it was a failure if MyParseDomCallback assigned something non-null to root node userdata
  if (parseResult == 1 && parseInfo.fakeRoot->UserData != 0)
  {
    parseResult = 0;
    if (errorBuffer != 0)
    {
      sprintf_s(errorBuffer, errorBufferLength, "most likely out of memory"); // it's just a guess
    }
  }

  // fire the callback if parsing didn't fail
  if (parseResult)
  {
    callback(parseInfo.fakeRoot, userData);
  }

  // free all allocated NateXmlNodes
  current = parseInfo.fakeRoot;
  while (current != 0)
  {
    // go to last leaf-most child
    count = NateXmlNode_GetCount(current);
    while (count > 0)
    {
      current = NateXmlNode_GetChild(current, count - 1);
      count = NateXmlNode_GetCount(current);
    }
    parent = current->Parent;
    // free it
    NateList_Uninit(&current->Children);
    free(current);
    // remove from parent
    if (parent != 0)
    {
      NateList_RemoveLast(&parent->Children);
    }
    // continue at parent
    current = parent;
  }

  return parseResult;
}

size_t NateXmlNode_GetCount(NateXmlNode * obj)
{
  return NateList_GetCount(&obj->Children);
}

NateXmlNode * NateXmlNode_GetChild(NateXmlNode * obj, size_t index)
{
  return (NateXmlNode*)NateList_GetPtr(&obj->Children, index);
}

// attribute name lookup is case sensitive
// returns null if name not found
char * NateXmlNode_GetAttribute(NateXmlNode * obj, const char * nameToFind)
{
  size_t i;
  char * name;
  char * value;
  char * attributes;

  attributes = obj->Attributes;
  for (i = 0; i < obj->AttributeCount; i++)
  {
    name = attributes;
    attributes += strlen(attributes) + 1;
    value = attributes;
    attributes += strlen(attributes) + 1;

    if (strcmp(name, nameToFind) == 0)
    {
      return value;
    }
  }

  return 0;
}