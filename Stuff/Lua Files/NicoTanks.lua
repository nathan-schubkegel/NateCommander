function Initialize_NicoTanks(state)

  state.Tanks =
    {
      Nate = {
               Name = "Nate",
               Position = {X = 0, Y = 0}, -- within state.WorldLeft/Top/Width/Height
               FaceAngle = 0, -- degrees, 0 means "looking down the positive X axis (right)"
               DesiredAngle = 0, -- degrees
               ThrottleStrength = 0, -- between 0 and 100, like a percentage
               Velocity = {
                            Angle = 0, -- degrees
                            Magnitude = 0, -- between 0 and 2 world units per 20ms tick
                          },               -- (that's a top speed of 100 world units per second)
               IsShootButtonHeldDown = false,
               TicksSinceLastShot = 1000000, -- ensures user can shoot immediately when game starts
               Keys = {Up = 'w', Down = 's', Left = 'a', Right = 'd', Shoot = 'LCTRL'},
             },
      Nico = {
               Name = "Nico", 
               Position = {X = 0, Y = 0}, -- within state.WorldLeft/Top/Width/Height
               FaceAngle = 0, -- degrees, 0 means "looking down the positive X axis (right)"
               DesiredAngle = 0, -- degrees
               ThrottleStrength = 0, -- between 0 and 100, like a percentage
               Velocity = {
                            Angle = 0, -- degrees
                            Magnitude = 0, -- between 0 and 2 world units per 20ms tick
                          },               -- (that's a top speed of 100 world units per second)
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
  
  -- this is in game axes, where X is -west/+east and Y is -south/+north
  state.CameraFocalPoint = { X = (state.WorldLeft + state.WorldWidth / 2),
                             Y = (state.WorldTop + state.WorldHeight / 2), }
                             
  state.WindowSize = { X = 500, Y = 500 } -- just guessing at window size initially

  state.Mashes =
    {
      -- TODO: actually load this info and use it
      --Tank = C_NateMash_LoadFromColladaResourceFile("nicotank.dae"),
      --Rock = C_NateMash_LoadFromColladaResourceFile("nicorock.dae"),
    }
    
  -- TODO: find a way to gather this from the NateMash
  state.DistanceFromTankCenterToSnoutTip = 20

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

  C_RegisterKeyResetHandler("HandleKeyEvent(Reset)",
    function(state, e) 
      HandleKeyEvent(state, e, "HandleKeyReset_", nil)
    end,
    nil)
  
  C_RegisterMouseMotionHandler("HandleMouseMotion", HandleMouseMotion)
    
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
    -- handlerMethodNamePrefix is like "HandleKeyDown_" or "HandleKeyUp_" or "HandleKeyReset_"
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
  
  if state.SDLK_KeysDown[keys.Up] then
    C_PolarVector2dAdd(desiredDirection, {Angle = 90, Magnitude = 100})
    inputProvided = true
  end
  
  if state.SDLK_KeysDown[keys.Down] then
    C_PolarVector2dAdd(desiredDirection, {Angle = 270, Magnitude = 100})
    inputProvided = true
  end
  
  if state.SDLK_KeysDown[keys.Left] then
    C_PolarVector2dAdd(desiredDirection, {Angle = 180, Magnitude = 100})
    inputProvided = true
  end
  
  if state.SDLK_KeysDown[keys.Right] then
    C_PolarVector2dAdd(desiredDirection, {Angle = 0, Magnitude = 100})
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
  
  tank.IsShootButtonHeldDown = state.SDLK_KeysDown[keys.Shoot]
end

function Clamp(value, min, max)
  if value < min then
    value = min
  end
  
  if value > max then
    value = max
  end
  
  return value;
end

function HandleMouseMotion(state, e)
  -- available event data:
  -- e.ChangeX, how much the mouse moved since the previous event
  -- e.ChangeY
  -- e.PositionX, absolute mouse position, (0,0 is top left)
  -- e.PositionY
  
  local xPercent = e.PositionX / state.WindowSize.X
  
  -- mouse events Y axis grows positive as mouse moves down the window
  -- but game world coordinates north is Y axis+ (the opposite)
  -- so compensate
  local yPercent = (state.WindowSize.Y - e.PositionY) / state.WindowSize.Y
  
  state.CameraFocalPoint = 
    {
      -- theoretically don't need clamping if state.WindowSize is correct
      -- but I can't always trust it. Sometimes it's going to be stale for a tick.
      X = Clamp(state.WorldLeft + state.WorldWidth * xPercent, state.WorldLeft, state.WorldLeft + state.WorldWidth),
      Y = Clamp(state.WorldTop + state.WorldHeight * yPercent, state.WorldTop, state.WorldTop + state.WorldHeight)
    }
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
  
  -- advance game state for every 1/50th of a second that goes by
  -- until it's current with NOW
  while msCount > 20 do
    msCount = msCount - 20;
    
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
    -- 1 rotation per 4 seconds
    -- 360 degrees per 4000 ms
    -- 1.8 degrees per 20ms tick
    tank.FaceAngle = RotateTowardAngle(tank.FaceAngle, tank.DesiredAngle, 1.8)
    
    -- convert all the tank's momentum into the direction it's facing
    -- (I guess it only moves forward for now)
    tank.Velocity.Angle = tank.FaceAngle
    
    if tank.ThrottleStrength < 10 then
      -- the tank isn't trying to go (deadzone = 10 percent throttle strength)
      -- so drag it to a stop within 700ms (35 ticks at 20ms)
      -- full speed is 100 units per second (2 units per 1 tick at 20ms)
      -- decceleration is (that speed) / (the time to achieve it) == (2 units per tick / 35 ticks)
      tank.Velocity.Magnitude = tank.Velocity.Magnitude - (2 / 35);
    else
      -- boost the tank in the direction it's facing
      -- the tank velocity should increase to full speed within 1200ms (60 ticks at 20ms)
      -- full speed is 100 units per second (2 units per 1 tick at 20ms)
      -- acceleration is (that speed) / (the time to achieve it) == (2 units per tick / 60 ticks)
      tank.Velocity.Magnitude = tank.Velocity.Magnitude + (tank.ThrottleStrength / 100) * (2 / 60)
    end

    -- no going backward, no going too fast
    tank.Velocity.Magnitude = Clamp(tank.Velocity.Magnitude, 0, 100)

    -- change the tank's position by the velocity
    local newPosition = C_CartesianVector2dAdd(tank.Position, 
      C_PolarVector2dToCartesian(tank.Velocity))

    -- keep the tank inside the world battlefield
    newPosition.X = Clamp(newPosition.X, state.WorldLeft, state.WorldLeft + state.WorldWidth);
    newPosition.Y = Clamp(newPosition.Y, state.WorldTop, state.WorldTop + state.WorldHeight);
    
    tank.Position = newPosition;
    
    -- TODO: collisions =)
  end
end

function RotateTowardAngle(currentAngle, desiredAngle, amount)
  local d1 = GetAbsAnglesBetween(currentAngle, desiredAngle)
  if d1 < math.abs(amount) then
    return desiredAngle
  end
  
  local d2 = GetAbsAnglesBetween(currentAngle + amount, desiredAngle)
  local d3 = GetAbsAnglesBetween(currentAngle - amount, desiredAngle)
  if d2 < d3 then
    return currentAngle + amount
  else
    return currentAngle - amount
  end
end

function GetAbsAnglesBetween(a1, a2)
  -- add or subtract 360 to a1 until it's as close to a2 as possible
  local best = math.abs(a2 - a1)
  local next = math.abs(a2 - (a1 + 360))
  while next < best do
    best = next
    a1 = a1 + 360
  end
  next = math.abs(a2 - (a1 - 360))
  while next < best do
    best = next
    a1 = a1 - 360
  end
  return best
end

function TryToShootGuns(state)
  for name, tank in pairs(state.Tanks) do
    if tank.IsShootButtonHeldDown then
      -- shoot a new bullet every 400 ms (once per 20 ticks)
      if tank.TicksSinceLastShot >= 20 then
      
        -- don't let tank fire again for a while
        tank.TicksSinceLastShot = 0

        local bullet = {
          -- start the bullet at the tank's snout        
          Position = C_CartesianVector2dAdd(tank.Position, C_PolarVector2dToCartesian(
            {
              Angle = tank.FaceAngle,
              Magnitude = state.DistanceFromTankCenterToSnoutTip
            })),

          -- move the bullet in the direction the tank is facing
          -- at 200 units per second (4 units per 20ms tick)
          -- plus the tank's current velocity (just like asteroids)
          Velocity = C_PolarVector2dAdd(
            { Angle = tank.FaceAngle, Magnitude = 4 },
            tank.Velocity),
          
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
    bullet.Position = C_CartesianVector2dAdd(bullet.Position, C_PolarVector2dToCartesian(bullet.Velocity))
    -- TODO: if bullet is colliding with tank, blow it up
    
    -- TODO: check for the whole path the tank might have traversed
    
    -- after bullets have been around for 2 seconds (100 ticks), destroy them
    if bullet.TicksAlive >= 100 then
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
  
  -- set the camera to look down on the playing field
  C_SetView_CameraLookingAtPoint_FromDistance_AtAngle(
    -- graphics axes are in terms of "being a person standing on the ground looking in -Z direction"
    -- game axes are in terms of "looking down on a town of people, oriented by a compass"

    -- FocalPointX (graphics X axis)
    -- graphics X axis is -left/+right of "looking into -Z direction"
    -- that is the game X axis -west/+east
    state.CameraFocalPoint.X,

    -- FocalPointY (graphics Y axis)
    -- graphics Y axis is -down/+up of "looking into -Z direction"
    -- that is the game Z axis -lower/+higher elevation
    0,

    -- FocalPointZ (graphics Z axis)
    -- graphics Z axis is -into monitor/+toward user of "looking into -Z direction"
    -- that is the game Y axis negated -(-south/+north)
    -state.CameraFocalPoint.Y,

    -- FocalPointDistance
    -- look down at the people from above
    30,

    -- LeftRightAngle
    -- graphics "LeftRight" means rotating around the up/down graphics Y axis
    -- graphics "LeftRight" 0 degrees means "looking into -Z direction" (into the monitor)
    -- in game terms we always want to look 90 degrees, north
    -- (in graphics terms that's 0 degrees)
    0,
    
    -- UpDownAngle
    -- -60 degrees means "looking down on the little people"
    -60)
    
  -- Clear the color and depth buffers.
  C_glClear(bit32.bor(0x00004000, 0x00000100)) -- GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
    
  -- Draw stuff
  C_DrawAxisLines();
  C_DrawYAngledCube(30); -- angle
  --C_NateMash_DrawUpright(durpMetronome, durpMetronomePosition, durpMetronomeRotation, durpMetronomeScale);
end
