
#include "MainEvents.h"
#include <SDL.h>
#include <assert.h>
#include <Windows.h>

/*
SDL_Event is a union of all event structures used in SDL, 
using it is a simple matter of knowing which union member 
relates to which event type.

Event type	Event Structure
SDL_ACTIVEEVENT	SDL_ActiveEvent
SDL_KEYDOWN/UP	SDL_KeyboardEvent
SDL_MOUSEMOTION	SDL_MouseMotionEvent
SDL_MOUSEBUTTONDOWN/UP	SDL_MouseButtonEvent
SDL_JOYAXISMOTION	SDL_JoyAxisEvent
SDL_JOYBALLMOTION	SDL_JoyBallEvent
SDL_JOYHATMOTION	SDL_JoyHatEvent
SDL_JOYBUTTONDOWN/UP	SDL_JoyButtonEvent
SDL_QUIT	SDL_QuitEvent
SDL_SYSWMEVENT	SDL_SysWMEvent
SDL_VIDEORESIZE	SDL_ResizeEvent
SDL_VIDEOEXPOSE	SDL_ExposeEvent
SDL_USEREVENT	SDL_UserEvent
*/

void MainEvents_HandleSdlEvent(SDL_Event *sdlEvent)
{
  switch (sdlEvent->type)
  {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
    case SDL_MOUSEMOTION:
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
    case SDL_JOYAXISMOTION:
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
      //Input::HandleEvent(sdlEvent);
      break;

    // Joystick trackball motion
    case SDL_JOYBALLMOTION:
    // Joystick hat position change
    case SDL_JOYHATMOTION:    /**<  */
      break; // not handling these now

    // User-requested quit
    case SDL_QUIT:
      // TODO: save stuff on quit?
      exit(0);
      break;

    // Application loses/gains visibility
    case SDL_ACTIVEEVENT:
      break;

    //System specific event
    case SDL_SYSWMEVENT:
      break;

    // User resized video mode
    case SDL_VIDEORESIZE:
      break;

    // Screen needs to be redrawn
    case SDL_VIDEOEXPOSE:
      break;

    // Events SDL_USEREVENT = 24 through SDL_MAXEVENTS-1 = 31 are for your use
    // There are also some reserved events
    default:
      break;
  }
}

int gIsProcessingSdlEvent;
SDL_Event gSdlEvent;

void MainEvents_PollEventsForever()
{
  assert(!gIsProcessingSdlEvent);
  if (!gIsProcessingSdlEvent)
  {
    gIsProcessingSdlEvent = 1;

again:
    while (SDL_PollEvent(&gSdlEvent))
    {
      MainEvents_HandleSdlEvent(&gSdlEvent);
    }
    Sleep(0); // give up execution to other threads that might want it
    goto again;
  }
}
