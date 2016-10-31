/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "GameControllerMapper.h"

#include "SDL.h"
#include "FatalErrorHandler.h"

void GameControllerMapper_MapJoystick(int joystickIndex)
{
  SDL_JoystickGUID jGUID;
  char guidString[33]; // 16 bytes of GUID as 32 hex chars plus null terminator
  char * name;

  // do we not yet have a controller mapping for this joystick?
  if (!SDL_IsGameController(joystickIndex))
  {
    jGUID = SDL_JoystickGetDeviceGUID( joystickIndex );
    
    guidString[0] = 0;
    SDL_JoystickGetGUIDString(jGUID, guidString, 33);
    guidString[32] = 0;

    name = SDL_JoystickNameForIndex(joystickIndex);
    if (name == 0 || name[0] == 0)
    {
      name = "Gamepad";
    }

    char * buffer = malloc(10000);
    if (buffer == 0) FatalError_OutOfMemory("gamepad mapping buffer");


    // bX is joystick button index X
    // hX.Y is hat index X with value Y
    // aX is joystick axis index X
    // a:b1,b:b2,y:b3,x:b0,start:b9,guide:b12,back:b8,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftshoulder:b4,rightshoulder:b5,leftstick:b10,rightstick:b11,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b6,righttrigger:b7
    SDL_GameControllerAddMapping();

    SDL_GameControllerGetStringForButton(SDL_GameControllerButton
      /*
          SDL_CONTROLLER_BUTTON_A,
    SDL_CONTROLLER_BUTTON_B,
    SDL_CONTROLLER_BUTTON_X,
    SDL_CONTROLLER_BUTTON_Y,
    SDL_CONTROLLER_BUTTON_BACK,
    SDL_CONTROLLER_BUTTON_GUIDE,
    SDL_CONTROLLER_BUTTON_START,
    SDL_CONTROLLER_BUTTON_LEFTSTICK,
    SDL_CONTROLLER_BUTTON_RIGHTSTICK,
    SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
    SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
    SDL_CONTROLLER_BUTTON_DPAD_UP,
    SDL_CONTROLLER_BUTTON_DPAD_DOWN,
    SDL_CONTROLLER_BUTTON_DPAD_LEFT,
    SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
    */);

    SDL_GameControllerGetStringForAxis(SDL_GameControllerAxis
      /*
          SDL_CONTROLLER_AXIS_LEFTX,
    SDL_CONTROLLER_AXIS_LEFTY,
    SDL_CONTROLLER_AXIS_RIGHTX,
    SDL_CONTROLLER_AXIS_RIGHTY,
    SDL_CONTROLLER_AXIS_TRIGGERLEFT,
    SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
    */);

    SDL_GameControllerOpen(joystickIndex);
    SDL_JoystickGetGUIDString();

    

    SDL_GameControllerMapping();
  }

  // map first axis to X1
  // map second axis to Y1
  // map third axis to X2
  // map fourth axis to Y2
  // map up to 20 buttons
  // 
}