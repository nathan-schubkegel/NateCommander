#ifndef NATE_COMMANDER_STRING_EATER
#define NATE_COMMANDER_STRING_EATER

#include <string.h>

struct StringEater
{
  const char * string;
  size_t maxSize;
  size_t currentPos;
  int errorState; // 0 = OK, 1 = error
};
typedef struct StringEater StringEater;

void StringEater_Init(StringEater * eater, const char * string, size_t maxSize);
Uint8 StringEater_Uint8(StringEater * eater);
Uint16 StringEater_Uint16(StringEater * eater);
char StringEater_Char(StringEater * eater);

int StringEater_Success(const char * buffer, size_t maxSize, size_t * currentPos, Uint8 * result);

#endif