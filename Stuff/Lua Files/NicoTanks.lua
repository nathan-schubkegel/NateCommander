--
-- This is free and unencumbered software released into the public domain under The Unlicense.
-- You have complete freedom to do anything you want with the software, for any purpose.
-- Please refer to <http://unlicense.org/>
--

function Initialize_NicoTanks(state)

  state.Tanks =
    {
      Nate = {
               Name = "Nate",
               Position = {X = -25, Y = -25}, -- within state.WorldLeft/Top/Width/Height
               SizeMultiplier = 5, -- scale tank by this much
               FaceAngle = 90, -- degrees, 0 means "looking down the positive X axis (right)"
               DesiredAngle = 90, -- degrees
               ThrottleStrength = 0, -- between 0 and 100, like a percentage
               Velocity = {
                            Angle = 0, -- degrees
                            Magnitude = 0, -- world units per tick
                          },
               IsShootButtonHeldDown = false,
               TicksSinceLastShot = 1000000, -- ensures user can shoot immediately when game starts
               Keys = {Up = 'w', Down = 's', Left = 'a', Right = 'd', Shoot = 'LCTRL'},
             },
      Nico = {
               Name = "Nico", 
               Position = {X = 25, Y = 25}, -- within state.WorldLeft/Top/Width/Height
               SizeMultiplier = 5, -- scale tank by this much
               FaceAngle = 270, -- degrees, 0 means "looking down the positive X axis (right)"
               DesiredAngle = 270, -- degrees
               ThrottleStrength = 0, -- between 0 and 100, like a percentage
               Velocity = {
                            Angle = 0, -- degrees
                            Magnitude = 0, -- world units per tick
                          },
               IsShootButtonHeldDown = false,
               TicksSinceLastShot = 1000000, -- ensures user can shoot immediately when game starts
               Keys = {Up = 'UP', Down = 'DOWN', Left = 'LEFT', Right = 'RIGHT', Shoot = 'SPACE'},
             },
    }
    
  state.Bullets = {}

  state.WorldLeft = -50
  state.WorldTop = -50
  state.WorldWidth = 100
  state.WorldHeight = 100
  
  state.WorldTime = C_MsCounter_Create()
  C_MsCounter_Reset(state.WorldTime)
  state.WorldTimeLeftoverMs = 0
  state.Paused = false
  
  -- this is in game degrees
  -- 10 LeftRight degrees is looking a little left of "down the X axis"
  -- -30 UpDown degrees is looking down on the stuff
  state.CameraLeftRightAngle = 90
  state.CameraUpDownAngle = -60
  state.CameraDistance = 50
  state.SpinnyBoxZ = 0
                             
  -- this is in game axes, where X is -west/+east and Y is -south/+north
  state.WindowSize = { X = 500, Y = 500 } -- just guessing at window size initially
  state.Mouse = 
    {
      Position = { X = (state.WindowSize.X / 2),
                   Y = (state.WindowSize.Y / 2) },  -- just guessing a mouse position initially
      ButtonsDown = {},
    };

  state.Mashes =
    {
      Tank = C_NateMash_LoadFromColladaResourceFile("nicotank.dae"),
      Rock = C_NateMash_LoadFromColladaResourceFile("nicorock.dae"),
    }

  -- game constants
  state.GameConstants = {}

  -- TODO: find a way to gather this from the NateMash
  state.GameConstants.DistanceFromTankCenterToSnoutTipInMeshUnits = 1
  state.GameConstants.TankRadiusInMeshUnits = 0.5
  state.GameConstants.BulletRadiusInMeshUnits = 1.0

  -- Time constants
  state.GameConstants.MsPerTick = 20
  state.GameConstants.TicksPerSecond = 1000 / state.GameConstants.MsPerTick

  -- Tank speed
  state.GameConstants.TankMaxSpeedInUnitsPerSecond = 30
  state.GameConstants.TankMaxSpeedInUnitsPerTick =
        state.GameConstants.TankMaxSpeedInUnitsPerSecond / state.GameConstants.TicksPerSecond

  -- Tank acceleration
  state.GameConstants.TankAccelerationTimeMs = 800 -- means "it takes this long to reach max speed, in ms"
  state.GameConstants.TankAccelerationTimeTicks = -- means "it takes this long to reach max speed, in ticks"
        state.GameConstants.TankAccelerationTimeMs / state.GameConstants.MsPerTick
  state.GameConstants.TankMaxAccelerationPerTick =
        state.GameConstants.TankMaxSpeedInUnitsPerTick / state.GameConstants.TankAccelerationTimeTicks

  -- Tank deceleration
  state.GameConstants.TankDecelerationTimeMs = 500 -- means "it takes this long to stop rolling, in ms"
  state.GameConstants.TankDecelerationTimeTicks = -- means "it takes this long to stop rolling, in ticks"
        state.GameConstants.TankDecelerationTimeMs / state.GameConstants.MsPerTick
  state.GameConstants.TankDecelerationPerTick = 
        state.GameConstants.TankMaxSpeedInUnitsPerTick / state.GameConstants.TankDecelerationTimeTicks

  -- Tank rotation speed
  state.GameConstants.TankRotationTimeMs = 1200 -- means "it takes this long to rotate, in ms"
  state.GameConstants.TankRotationTimeTicks = -- means "it takes this long to rotate, in ticks"
        state.GameConstants.TankRotationTimeMs / state.GameConstants.MsPerTick
  state.GameConstants.TankRotationPerTick = 360 / state.GameConstants.TankRotationTimeTicks

  -- Tank shoot frequency
  state.GameConstants.TankShootFrequencyMs = 400 -- means "this much time between shots, in ms"
  state.GameConstants.TankShootFrequencyTicks = -- means "this much time between shots, in ticks"
        state.GameConstants.TankShootFrequencyMs / state.GameConstants.MsPerTick

  -- Bullet speed
  state.GameConstants.BulletSpeedInUnitsPerSecond = 80
  state.GameConstants.BulletSpeedInUnitsPerTick =
        state.GameConstants.BulletSpeedInUnitsPerSecond / state.GameConstants.TicksPerSecond

  -- Bullet lifetime, means "if it doesn't collide with anything in this amount of time, it goes away"
  state.GameConstants.BulletLifeTimeMs = 2000 -- "in ms"
  state.GameConstants.BulletLifeTimeTicks = -- "in ticks"
        state.GameConstants.BulletLifeTimeMs / state.GameConstants.MsPerTick

  -- Bullet misc values
  state.GameConstants.BulletSizeMultiplier = 0.8
  state.GameConstants.BulletHeadOverHeelsAnglesPerSecond = 560
  state.GameConstants.BulletHeadOverHeelsAnglesPerTick = 
        state.GameConstants.BulletHeadOverHeelsAnglesPerSecond * state.GameConstants.MsPerTick / 1000

  -- register event handlers for every key
  C_RegisterKeyDownHandler("HandleKeyEvent(Down)", 
    function(state, e) 
      HandleKeyEvent(state, e, "HandleKeyDown_", true)
    end,
    nil)
    
  C_RegisterKeyUpHandler("HandleKeyEvent(Up)",
    function(state, e) 
      HandleKeyEvent(state, e, "HandleKeyUp_", nil)
    end,
    nil)

  C_RegisterMouseMotionHandler("HandleMouseMotion", HandleMouseMotion)
  C_RegisterMouseDownHandler("HandleMouseDown", HandleMouseDown, nil)
  C_RegisterMouseUpHandler("HandleMouseUp", HandleMouseUp, nil)
  C_RegisterMouseWheelHandler("HandleMouseWheel", HandleMouseWheel)
    
  state.SDLK_Keys = 
    {
      ESCAPE = 27,
      a = 97,
      s = 115,
      d = 100,
      w = 119,
      LCTRL = 1073742048,
      DOWN = 1073741905,
      UP = 1073741906,
      LEFT = 1073741904,
      RIGHT = 1073741903,
      SPACE = 32,
      PAUSE = 1073741896,
    }
  
  state.SDLK_KeyNames = {}
  for keyName, keyValue in pairs(state.SDLK_Keys) do
    state.SDLK_KeyNames[keyValue] = keyName
  end

  state.SDLK_KeysDown = {}
end

function HandleKeyEvent(state, e, handlerMethodNamePrefix, keyDown)
  -- record whether the key is now down
  state.SDLK_KeysDown[e.keysym] = keyDown

  -- record whether the key is now down, by key name
  local keyName = state.SDLK_KeyNames[e.keysym]
  if keyName ~= nil then
    state.SDLK_KeysDown[keyName] = keyDown
  end
  
  for name, tank in pairs(state.Tanks) do
    UpdateTankForPressedButtons(state, tank)
  end

  if keyName ~= nil then
    -- handlerMethodNamePrefix is like "HandleKeyDown_" or "HandleKeyUp_"
    local keyHandler = _G[handlerMethodNamePrefix .. keyName]
    if keyHandler ~= nil then
      keyHandler(state, e)
    end
  end
end

function HandleKeyDown_ESCAPE(state, e)
  C_Exit(0)
end

function HandleKeyDown_PAUSE(state, e)
  -- TODO: play a sound like in Mario 3 and don't let this swap until the sound is complete
  state.Paused = not state.Paused
end

function UpdateTankForPressedButtons(state, tank)
  local desiredDirection = {Angle = 0, Magnitude = 0}
  local inputProvided = false
  local keys = tank.Keys;
  
  -- update magnitude and angle for keys pressed
  -- Magnitude is between 0 and 100 like a percent, ultimately it goes into tank.ThrottleStrength
  
  if state.SDLK_KeysDown[keys.Up] then
    desiredDirection = C_PolarVector2dAdd(desiredDirection, {Angle = 90, Magnitude = 100})
    inputProvided = true
  end
  
  if state.SDLK_KeysDown[keys.Down] then
    desiredDirection = C_PolarVector2dAdd(desiredDirection, {Angle = 270, Magnitude = 100})
    inputProvided = true
  end
  
  if state.SDLK_KeysDown[keys.Left] then
    desiredDirection = C_PolarVector2dAdd(desiredDirection, {Angle = 180, Magnitude = 100})
    inputProvided = true
  end
  
  if state.SDLK_KeysDown[keys.Right] then
    desiredDirection = C_PolarVector2dAdd(desiredDirection, {Angle = 0, Magnitude = 100})
    inputProvided = true
  end

  -- no cheating by going extra fast by going diagonal
  if desiredDirection.Magnitude > 100 then
    desiredDirection.Magnitude = 100
  end
  
  if not inputProvided then
    tank.DesiredAngle = tank.FaceAngle
    tank.ThrottleStrength = 0
  else
    tank.DesiredAngle = desiredDirection.Angle
    tank.ThrottleStrength = desiredDirection.Magnitude
  end
  
  tank.IsShootButtonHeldDown = state.SDLK_KeysDown[keys.Shoot] == true
end

function Clamp(value, min, max)
  if value < min then
    value = min
  end
  
  if value > max then
    value = max
  end
  
  return value
end

function NormalizeAngle(value)
  -- TODO: use modulo to guarantee performance
  while value < 0 do
    value = value + 360
  end
  
  while value > 360 do
    value = value - 360
  end
  
  return value
end

function HandleMouseMotion(state, e)
  -- available event data:
  -- e.ChangeX, how much the mouse moved since the previous event
  -- e.ChangeY
  -- e.PositionX, absolute mouse position, (0,0 is top left)
  -- e.PositionY
  -- e.Buttons, table of numbers representing SDL mouse buttons
  --     SDL_BUTTON_LEFT     1
  --     SDL_BUTTON_MIDDLE   2
  --     SDL_BUTTON_RIGHT    3
  --     SDL_BUTTON_X1       4
  --     SDL_BUTTON_X2       5
  
  local pos = state.Mouse.Position
  pos.X = e.PositionX
  pos.Y = e.PositionY
  
  -- while left button is down, rotate camera by the changed amount
  if e.Buttons[1] ~= nil then
    local xPercent = -e.ChangeX / 500
    local yPercent = -e.ChangeY / 500

    state.CameraLeftRightAngle = NormalizeAngle(state.CameraLeftRightAngle + (xPercent * 360))
    state.CameraUpDownAngle = Clamp(state.CameraUpDownAngle + (yPercent * 360), -90, 90)
  end
  
  -- while right button is down, change camera distance
  if e.Buttons[3] ~= nil then
    state.CameraDistance = Clamp(state.CameraDistance - e.ChangeY, 0, 100)
  end
end

function HandleMouseDown(state, e)
  -- available event data:
  -- e.PositionX, absolute mouse position, (0,0 is top left)
  -- e.PositionY
  -- e.Button, number representing changed SDL mouse button
  --     SDL_BUTTON_LEFT     1
  --     SDL_BUTTON_MIDDLE   2
  --     SDL_BUTTON_RIGHT    3
  --     SDL_BUTTON_X1       4
  --     SDL_BUTTON_X2       5
  -- e.Pressed, true if pressed, false if released
  -- e.Clicks, number of clicks so far (1, 2, etc)
  state.Mouse.ButtonsDown[e.Button] = true;
  
  -- TODO:
  -- when user clicks right mouse button down somewhere,
  -- paint a target there
  if e.Button == 3 then
  end
end

function HandleMouseUp(state, e)
  -- available event data: same as MouseDown
  state.Mouse.ButtonsDown[e.Button] = nil;
  
  -- TODO:
  -- when user releases right mouse button somewhere,
  -- paint another target there, and draw a line between them
  if e.Button == 3 then
  end
end

function HandleMouseWheel(state, e)
  -- available event data:
  -- e.ScrollX, relative x scroll movement
  -- e.ScrollY
  
  -- move spinny box up/down with scroll wheel
  state.SpinnyBoxZ = Clamp(state.SpinnyBoxZ + (e.ScrollY / 10), -50, 50)
end

function Process(state)
  -- determine how much time has passed since the last call to Process
  local msCountBefore = C_MsCounter_GetCount(state.WorldTime)
  if not state.Paused then
    -- update our sense of time
    C_MsCounter_Update(state.WorldTime)
  else
    -- maintain a frozen sense of time
    C_MsCounter_ResetToCurrentCount(state.WorldTime)
  end
  local msCountAfter = C_MsCounter_GetCount(state.WorldTime)
  local msCount = (msCountAfter - msCountBefore) + state.WorldTimeLeftoverMs
  state.WorldTimeLeftoverMs = 0

  -- if we start getting behind then don't attempt to crunch more than half a second
  -- we'll just skip the extra for now
  if msCount > 500 then
    msCount = 500
  end
  
  -- advance game state for every tick that goes by
  -- until it's current with NOW
  while msCount > state.GameConstants.MsPerTick do
    msCount = msCount - state.GameConstants.MsPerTick;
    
    --C_NonFatalError('processing a loop')
    
    -- try to move tanks
    -- try to shoot guns    
    -- try to move bullets
    -- try to blow stuff up
    TryToMoveTanks(state)
    TryToShootGuns(state)
    TryToMoveBullets(state)
    TryToBlowStuffUp(state)
  end
  
  state.WorldTimeLeftoverMs = msCount
end

function TryToMoveTanks(state)
  for name, tank in pairs(state.Tanks) do
    -- if the user wants to rotate, let him rotate toward the angle he's desiring
    tank.FaceAngle = RotateTowardAngle(tank.FaceAngle, tank.DesiredAngle, state.GameConstants.TankRotationPerTick)
    -- TODO: need to let the user do this slowly with gamepads
    
    -- convert all the tank's momentum into the direction it's facing
    -- (I guess it only moves forward for now)
    tank.Velocity.Angle = tank.FaceAngle
    
    if tank.ThrottleStrength < 10 then
      -- the tank isn't trying to go (deadzone = 10 percent throttle strength)
      -- so drag it to a stop
      tank.Velocity.Magnitude = tank.Velocity.Magnitude - state.GameConstants.TankDecelerationPerTick;
    else
      -- boost the tank in the direction it's facing, limited by throttle strength
      tank.Velocity.Magnitude = tank.Velocity.Magnitude + 
        (tank.ThrottleStrength / 100) * state.GameConstants.TankMaxAccelerationPerTick
    end
    -- TODO: need to let the user actually decelerate when ThrottleStrength is low

    -- no going backward, no going too fast
    tank.Velocity.Magnitude = Clamp(tank.Velocity.Magnitude, 0, state.GameConstants.TankMaxSpeedInUnitsPerTick)

    -- change the tank's position by the velocity
    local newPosition = C_CartesianVector2dAdd(tank.Position,
      C_PolarVector2dToCartesian(tank.Velocity))

    -- keep the tank inside the world battlefield
    local tankRadius = state.GameConstants.TankRadiusInMeshUnits * tank.SizeMultiplier;    
    newPosition.X = Clamp(newPosition.X,
        state.WorldLeft + tankRadius,
        state.WorldLeft + state.WorldWidth - tankRadius);
    newPosition.Y = Clamp(newPosition.Y,
        state.WorldTop + tankRadius,
        state.WorldTop + state.WorldHeight - tankRadius);

    tank.Position = newPosition;

    -- TODO: collisions =)
  end
end

function RotateTowardAngle(currentAngle, desiredAngle, amount)
  local d1 = C_GetAbsAnglesBetween(currentAngle, desiredAngle)
  if d1 < math.abs(amount) then
    return desiredAngle
  end
  
  local d2 = C_GetAbsAnglesBetween(currentAngle + amount, desiredAngle)
  local d3 = C_GetAbsAnglesBetween(currentAngle - amount, desiredAngle)
  if d2 < d3 then
    return currentAngle + amount
  else
    return currentAngle - amount
  end
end

function TryToShootGuns(state)
  for name, tank in pairs(state.Tanks) do

    -- advance tank's ability to shoot again soon
    tank.TicksSinceLastShot = tank.TicksSinceLastShot + 1

    if tank.IsShootButtonHeldDown then

      -- shoot a new bullet every once in a while
      if tank.TicksSinceLastShot >= state.GameConstants.TankShootFrequencyTicks then

        -- don't let tank fire again for a while
        tank.TicksSinceLastShot = 0

        local bullet = {
          -- start the bullet at the tank's snout        
          Position = C_CartesianVector2dAdd(tank.Position, C_PolarVector2dToCartesian(
            {
              Angle = tank.FaceAngle,
              Magnitude = state.GameConstants.DistanceFromTankCenterToSnoutTipInMeshUnits *
                             tank.SizeMultiplier,
            })),

          -- move the bullet in the direction the tank is facing
          -- plus the tank's current velocity (just like asteroids)
          Velocity = C_PolarVector2dAdd(
            { Angle = tank.FaceAngle, Magnitude = state.GameConstants.BulletSpeedInUnitsPerTick },
            tank.Velocity),

          -- record the direction it was originally intended to be shot
          FaceAngle = tank.FaceAngle,
          
          -- record who shot it
          Tank = tank,
          
          -- record how long it's been around
          -- (bullets drop out of the universe after 2 seconds)
          TicksAlive = 0,
        }

        -- add to global bullets list
        state.Bullets[bullet] = bullet
      end
    end
  end
end

function TryToMoveBullets(state)
  for bullet, bullet2 in pairs(state.Bullets) do

    -- TODO: if bullet is colliding with tank, blow it up
    -- TODO: check for the whole path the tank might have traversed

    -- move bullet
    bullet.Position = C_CartesianVector2dAdd(bullet.Position, C_PolarVector2dToCartesian(bullet.Velocity))
    
    -- after bullets have been around long enough, discard them
    bullet.TicksAlive = bullet.TicksAlive + 1
    if bullet.TicksAlive >= state.GameConstants.BulletLifeTimeTicks then
      state.Bullets[bullet] = nil
    end
  end
end

function TryToBlowStuffUp(state)
end

function Draw(state, e)
  -- These values are available:
  -- e.WindowWidth
  -- e.WindowHeight
  
  -- save WindowWidth and WindowHeight so mouse movement events can leverage it
  -- TODO: actually provide a way to ask the C host what are the window width/height
  --       rather than waiting for a render event to find out. Fortunately it only affects rendering!
  state.WindowSize = { X = e.WindowWidth, Y = e.WindowHeight }

  C_glMatrixMode(0x1701) -- GL_PROJECTION
  C_glLoadIdentity()

  C_gluPerspective(
    60, -- lense angle
    e.WindowWidth / e.WindowHeight, -- aspect ratio
    1.0, -- zNear
    1024.0) -- zFar

  -- determine the point between the two tanks
  local focalPoint = { X = (state.Tanks.Nate.Position.X + state.Tanks.Nico.Position.X) / 2,
                       Y = (state.Tanks.Nate.Position.Y + state.Tanks.Nico.Position.Y) / 2,
                       Z = 0 }

  -- TODO: be smarter
  local cameraDistance = (math.abs(state.Tanks.Nate.Position.X - state.Tanks.Nico.Position.X) + 
                         math.abs(state.Tanks.Nate.Position.Y - state.Tanks.Nico.Position.Y))

  -- graphics coordinates are in terms of
  -- "being a person standing on the ground looking forward" with
  -- -Z forward, +Z behind, -X left, +X right, +Y up, -Y down

  -- game coordinates are in terms of
  -- "looking down on a town of people, oriented by a compass" with
  -- -Z underground, +Z sky, -X west, +X east, +Y north, -Y south"
  
  -- blender models have same X/Y/Z as these game coordinates
  -- and I'm standardizing on blender models facing the X axis
  -- so 90 degrees rotation causes the model to look at y+ northward

  -- establish camera distance
  -- (on Z axis because at this point in the transformations, -Z still goes into the screen)
  C_glTranslate(0, 0, -state.CameraDistance)

  -- rotate everything -90 degrees around X axis
  -- so +Z axis points into sky and +Y axis points north. Then we can draw in game coordinates
  C_glRotate(-90, 1, 0, 0)

  -- rotate camera in the direction desired to look.
  -- negative because 'CameraUpDownAngle' is in terms of "where is the camera on the face of a globe"
  -- but really this matrix rotates what's drawn to give that appearance.
  C_glRotate(-state.CameraUpDownAngle, 1, 0, 0)
  
  -- rotate everything 90 degrees around Z axis (up/down axis)
  -- so +X axis points into the screen. (That puts it in game coordinates)
  C_glRotate(90, 0, 0, 1)  
  
  -- rotate camera in the direction desired to look.
  -- negative because 'CameraLeftRightAngle' is in terms of "where is the camera on the face of a globe"
  -- but really this matrix rotates what's drawn to give that appearance.
  C_glRotate(-state.CameraLeftRightAngle, 0, 0, 1)  
  
  -- scoot camera around the map
  -- negative because really this moves what's drawn
  C_glTranslate(
    -focalPoint.X,
    -focalPoint.Y,
    0)

  -- Clear the color and depth buffers.
  C_glClear(bit32.bor(0x00004000, 0x00000100)) -- GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT

  -- Draw world objects
  C_glMatrixMode(0x1700) -- GL_MODELVIEW

  -- Draw axis lines and a cube in the middle so I know where the middle is
  C_glLoadIdentity()  
  C_DrawAxisLines()
  C_glScale(5, 5, 5)
  -- (make the cube spin around at some speed)
  C_glRotate(((C_MsCounter_GetCount(state.WorldTime) % 3500) / 3500) * 360, 0, 0, 1)
  -- (make the cube hover based on scroll wheel)
  C_glTranslate(0, 0, state.SpinnyBoxZ)
  C_DrawRainbowCube()

  for name, tank in pairs(state.Tanks) do
    C_glLoadIdentity()

    C_glTranslate(
      tank.Position.X,
      tank.Position.Y,
      0)

    C_glScale(
      tank.SizeMultiplier,
      tank.SizeMultiplier,
      tank.SizeMultiplier)

    C_glRotate(tank.FaceAngle, 0, 0, 1)

    C_NateMash_Draw(state.Mashes.Tank)
  end

  for bullet, bullet2 in pairs(state.Bullets) do
    C_glLoadIdentity()

    C_glTranslate(
      bullet.Position.X,
      bullet.Position.Y,
      0)

    C_glScale(
      state.GameConstants.BulletSizeMultiplier,
      state.GameConstants.BulletSizeMultiplier,
      state.GameConstants.BulletSizeMultiplier)

    -- make the bullet face the direction it was originally shot
    C_glRotate(bullet.FaceAngle, 0, 0, 1)

    -- make the bullet spin head-over-heels in the direction it was originally shot
    -- (rotating on Y axis because this transformation occurs "before" the bullet is
    --  faced the direction it was originally shot)
    C_glRotate(bullet.TicksAlive * state.GameConstants.BulletHeadOverHeelsAnglesPerTick, 0, 1, 0)

    C_DrawRainbowCube()
  end

  --C_glLoadIdentity()
end
