/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "MeasuredString.h"
#include <assert.h>

void MeasuredString_AllocFromAscii(MeasuredString * this, char * zeroTerminatedAsciiString)
{
  size_t len;
  Utf32Char * data;
  Utf32Char * end;
  char * here;

  len = strlen(zeroTerminatedAsciiString);

  if (len == 0)
  {
    this->Data = NULL;
    this->Length = 0;
  }
  else
  {
    data = malloc(len * sizeof(Utf32Char));
    this->Data = data;
    this->Length = len;
    end = data + len;
    here = zeroTerminatedAsciiString;
    while (data < end)
    {
      *data = *here;
      data++;
      here++;
    }
  }
}

void MeasuredString_AllocFromUtf8(MeasuredString * this, char * utf8Data, Uint32 utf8DataLength)
{
  char * here;
  char * end;
  int numBytesInChar;
  Uint32 numCodePoints;
  int lastWasInvalid;
  Utf32Char * data;

  assert(sizeof(char) == 1);

  // first count the number of codepoints
  numCodePoints = 0;
  lastWasInvalid = 0;
  here = utf8Data;
  end = utf8Data + utf8DataLength;
  while (here < end)
  {
    numBytesInChar = utf8_validate_char(here, end);
    if (numBytesInChar == 0)
    {
      if (!lastWasInvalid) // only show 1 invalid character for a series of invalid ones
      {
        numCodePoints++;
      }
      lastWasInvalid = 1; // true
      here++;
    }
    else
    {
      numCodePoints++;
      lastWasInvalid = 0; // false
      here += numBytesInChar;
    }
  }

  if (numCodePoints == 0)
  {
    this->Data = NULL;
    this->Length = 0;
  }
  else
  {
    // allocate space
    this->Data = malloc(numCodePoints * sizeof(Utf32Char));
    this->Length = numCodePoints;
    data = this->Data;

    // then put the codepoints in an array
    lastWasInvalid = 0;
    here = utf8Data;
    end = utf8Data + utf8DataLength;
    while (here < end)
    {
      numBytesInChar = utf8_validate_char(here, end);
      if (numBytesInChar == 0)
      {
        if (!lastWasInvalid) // only show 1 invalid character for a series of invalid ones
        {
          *data = '?';
          data++;
        }
        lastWasInvalid = 1; // true
        here++;
      }
      else
      {
        utf8_read_char(here, data);
        data++;
        lastWasInvalid = 0; // false
        here += numBytesInChar;
      }
    }
  }
}

void MeasuredString_AllocFromLength(MeasuredString * this, Uint32 numCodePoints)
{
  if (numCodePoints == 0)
  {
    this->Data = NULL;
    this->Length = 0;
  }
  else
  {
    this->Data = malloc(numCodePoints * sizeof(Utf32Char));
    memset(this->Data, 0, numCodePoints * sizeof(Utf32Char));
    this->Length = numCodePoints;
  }
}

void MeasuredString_Free(MeasuredString * this)
{
  // TODO: is this guard already performed by free?
  if (this->Data != NULL)
  {
    free(this->Data);
    this->Data = NULL;
  }
  
  this->Length = 0;
}