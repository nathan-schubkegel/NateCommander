#ifndef NATE_COMMANDER_FATAL_ERROR_HANDLER
#define NATE_COMMANDER_FATAL_ERROR_HANDLER

void FatalError(const char * message);
void FatalError_Sdl(const char * message);
void FatalError_OutOfMemory();

void NonFatalError(const char * message);
void NonFatalError_Sdl(const char * message);

#endif