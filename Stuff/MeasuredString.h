#ifndef NATE_COMMANDER_MEASURED_STRING_H
#define NATE_COMMANDER_MEASURED_STRING_H

#include "NateUtf8.h"

typedef struct MeasuredString
{
  Unicode_Codepoint_t * Data;
  Uint32 Length;
}
MeasuredString;

void MeasuredString_AllocFromAscii(MeasuredString * this, char * zeroTerminatedAsciiString);
void MeasuredString_AllocFromUtf8(MeasuredString * this, char * utf8Data, Uint32 utf8DataLength);
void MeasuredString_AllocFromLength(MeasuredString * this, Uint32 numCodePoints);
void MeasuredString_Free(MeasuredString * this);

#endif