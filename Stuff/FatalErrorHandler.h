#ifndef NATE_COMMANDER_FATAL_ERROR_HANDLER
#define NATE_COMMANDER_FATAL_ERROR_HANDLER

void FatalError(const char * message);
void FatalError2(const char * message, const char * message2);
void FatalError_Sdl(const char * message);
void FatalError_OutOfMemory();

void NonFatalError(const char * message);
void NonFatalError_Sdl(const char * message);

#define NateAssert(condition, message) if (!(condition)) { FatalError(#condition ", " message); }

#define BuildAssert(cond) \
	(void) sizeof(char [1 - 2*!(cond)]);

#define BuildAssertSize(expr, size) \
  (void) sizeof(char[1 - 50 * ((size) - (expr))]); \
  (void) sizeof(char[1 - 50 * ((expr) - (size))]);

int FatalError_IsDeliveringMessage();

#endif