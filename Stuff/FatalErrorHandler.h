/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#ifndef NATE_COMMANDER_FATAL_ERROR_HANDLER
#define NATE_COMMANDER_FATAL_ERROR_HANDLER

// google says this is how I can get __LINE__ to be a string
#define STRINGIFY1(s) #s
#define STRINGIFY2(s) STRINGIFY1(s)

#define FatalError(message) DoFatalError2((message), " at line " STRINGIFY2(__LINE__) " in " __FUNCTION__ " in file: " __FILE__)
#define FatalError2(message, message2) DoFatalError3((message), (message2), " at line " STRINGIFY2(__LINE__) " in " __FUNCTION__ " in file: " __FILE__)
#define FatalError3(message, message2, message3) DoFatalError4((message), (message2), (message3), " at line " STRINGIFY2(__LINE__) " in " __FUNCTION__ " in file: " __FILE__)
#define FatalError_Sdl(message) DoFatalError_Sdl2((message), " at line " STRINGIFY2(__LINE__) " in " __FUNCTION__ " in file: " __FILE__)
#define FatalError_OutOfMemory(message) DoFatalError_OutOfMemory2((message), " at line " STRINGIFY2(__LINE__) " in " __FUNCTION__ " in file: " __FILE__)
void DoFatalError2(const char * message, const char * message2);
void DoFatalError3(const char * message, const char * message2, const char * message3);
void DoFatalError4(const char * message, const char * message2, const char * message3, const char * message4);
void DoFatalError_Sdl2(const char * message, const char * message2);
void DoFatalError_OutOfMemory2(const char * message, const char * message2);

#define NonFatalError(message) DoNonFatalError2((message), " at line " STRINGIFY2(__LINE__) " in " __FUNCTION__ " in file: " __FILE__)
#define NonFatalError_Sdl(message) DoNonFatalError_Sdl2((message), " at line " STRINGIFY2(__LINE__) " in " __FUNCTION__ " in file: " __FILE__)
void DoNonFatalError2(const char * message, const char * message2);
void DoNonFatalError_Sdl2(const char * message, const char * message2);

// these need runtime evaluation
#define NateCheck0(condition) if (!(condition)) { FatalError(#condition); }
#define NateCheck0_Sdl(condition) if (!(condition)) { FatalError_Sdl(#condition); }
#define NateCheck(condition, message) if (!(condition)) { FatalError(#condition ", " message); }
#define NateCheck_Sdl(condition, message) if (!(condition)) { FatalError_Sdl(#condition ", " message); }
#define NateCheck2(condition, message1, message2) if (!(condition)) { FatalError2(#condition ", " message1, (message2)); }
#define NateCheck3(condition, message1, message2, message3) if (!(condition)) { FatalError3(#condition ", " message1, (message2), (message3)); }

// these are theoretically compiled out at release time
#define NateAssert0(condition) if (!(condition)) { FatalError(#condition); }
#define NateAssert(condition, message) if (!(condition)) { FatalError2(#condition ", ", (message)); }
#define NateAssert2(condition, message1, message2) if (!(condition)) { FatalError3(#condition ", ", (message1), (message2)); }

#define BuildAssert(cond) \
	(void) sizeof(char [1 - 2*!(cond)]);

#define BuildAssertSize(expr, size) \
  (void) sizeof(char[1 - 50 * ((size) - (expr))]); \
  (void) sizeof(char[1 - 50 * ((expr) - (size))]);

int FatalError_IsDeliveringMessage();

#endif