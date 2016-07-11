
function Initialize_SpinnyBoxApp(state)
  state.SpinnyBoxAngle = 0.0
  state.FloorZSpeed = 3.5 -- in world units per second
  state.FloorZOffset = 0.0 -- this gets update in MainApp_Process once per game tick
  state.FloorZOffsetDirection = nil -- I know this is basically a noop, but I like the clarity
  -- TODO: keep track of "when the user first pressed the key down" and "what the zOffset was then"
  -- so we can update without significant floating point arithmetic loss
  state.ViewAngleX = 0
  state.ViewAngleY = 0
  state.ElapsedTime = C_MsCounter_Create()
  C_MsCounter_Reset(state.ElapsedTime)
  state.ShouldRotate = true
  state.LastSim60thSecondCount = 0
  state.Mashes = {}
  state.Mashes.DurpMetronome = C_NateMash_LoadFromColladaResourceFile("two_boxes_parented.dae")
  
  -- register event handlers
  C_RegisterKeyDownHandler("HandleKeyDown_Escape", HandleKeyDown_Escape, 27) -- SDLK_ESCAPE
  C_RegisterKeyDownHandler("HandleKeyDown_Space", HandleKeyDown_Space, 32) -- SDLK_SPACE
  C_RegisterKeyDownHandler("HandleKeyDown_a", HandleKeyDown_a, 97) -- SDLK_a

  C_RegisterKeyDownHandler("HandleKeyDown_DownArrow", HandleKeyDown_DownArrow, 1073741905) -- SDLK_DOWN
  C_RegisterKeyUpHandler("HandleKeyUp_DownArrow", HandleKeyUp_DownArrow, 1073741905) -- SDLK_DOWN

  C_RegisterKeyDownHandler("HandleKeyDown_UpArrow", HandleKeyDown_UpArrow, 1073741906) -- SDLK_UP
  C_RegisterKeyUpHandler("HandleKeyUp_UpArrow", HandleKeyUp_UpArrow, 1073741906) -- SDLK_UP  
  
  C_RegisterMouseMotionHandler("HandleMouseMotion", HandleMouseMotion)
end

function HandleKeyDown_Escape(state, e)
  C_Exit(0)
end

function HandleKeyDown_Space(state, e)
  state.ShouldRotate = not state.ShouldRotate
end

function HandleKeyDown_a(state, e)
  C_NonFatalError("user-initiated non-fatal error")
end

function HandleKeyDown_DownArrow(state, e)
  state.FloorZOffsetDirection = "closer"
end

function HandleKeyUp_DownArrow(state, e)
  -- todo: do a better job gracefully handling spurious multiple keystrokes and "both keys held down"
  state.FloorZOffsetDirection = nil
end

function HandleKeyDown_UpArrow(state, e)
  state.FloorZOffsetDirection = "farther"
end

function HandleKeyUp_UpArrow(state, e)
  state.FloorZOffsetDirection = nil
end

function HandleMouseMotion(state, e)
  --state.ViewAngleX = state.ViewAngleX + e.relx
  --state.ViewAngleY = state.ViewAngleY + e.rely
  state.ViewAngleX = e.x
  state.ViewAngleY = e.y      
end

-- TODO: handle left and right arrows
--    if (keySym == 1073741903) then -- SDLK_RIGHT
--    elseif (keySym == 1073741904) then -- SDLK_LEFT
-- TODO: move window management into lua
-- TODO: add joystick input handling
-- TODO: add tablet input handling

function Process(state)
  if state.ShouldRotate then
    -- update our sense of time
    local msCount = C_MsCounter_Update(state.ElapsedTime)
    
    -- produce a full rotation every 2 seconds
    state.SpinnyBoxAngle = (msCount % 2000) * 360 / 2000
  else
    -- maintain a frozen sense of time
    C_MsCounter_ResetToCurrentCount(state.ElapsedTime)
  end
  
  -- also advance game state and the simulator for every 1/60th of a second that goes by
  do
    local msCount = C_MsCounter_GetCount(state.ElapsedTime)
    local num60thSeconds = (msCount / 1000) * 60;
    
    -- if we start getting behind (example: the simulation is taking too long to compute)
    -- then don't attempt to crunch more than half a second's worth of simulation
    -- we'll just skip the extra for now
    if state.LastSim60thSecondCount + 30 < num60thSeconds then
      state.LastSim60thSecondCount = num60thSeconds - 30
    end
    
    -- advance the simulation until it's current with NOW
    while state.LastSim60thSecondCount < num60thSeconds do
      state.LastSim60thSecondCount = state.LastSim60thSecondCount + 1
      
      -- move the floor if we're supposed to
      if state.FloorZOffsetDirection == "closer" then
        state.FloorZOffset = state.FloorZOffset + (state.FloorZSpeed / 60)
      elseif state.FloorZOffsetDirection == "farther" then
        state.FloorZOffset = state.FloorZOffset - (state.FloorZSpeed / 60)
      end
      
      -- advanced physics-controlled objects
      -- TODO: this cheats to deliver the new floorzoffset. does it deserve its own method?
      C_AdvanceGSIM(state.FloorZOffset)
    end
  end
end

function Draw(state)
  -- TODO: more engine, less hacks
  return state.SpinnyBoxAngle, state.FloorZOffset, state.ViewAngleX, state.ViewAngleY, state.Mashes.DurpMetronome
end
