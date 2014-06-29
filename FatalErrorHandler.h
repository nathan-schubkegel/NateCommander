#ifndef NATE_COMMANDER_FATAL_ERROR_HANDLER
#define NATE_COMMANDER_FATAL_ERROR_HANDLER

void FatalError(const wchar_t * message);
void FatalError(const wchar_t * message, const char * extraInfo);

#endif