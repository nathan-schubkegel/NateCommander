
function MainApp_Initialize(state)
  state.CurrentAngle = 0.0

  state.FloorZSpeed = 3.5 -- in world units per second
  state.FloorZOffset = 0.0 -- this gets update in MainApp_Process once per game tick
  state.FloorZOffsetDirection = nil -- I know this is basically a noop, but I like the clarity
  
  --state.KeyStates = {}
  --state.KeysWhenPressed = {}

  state.ElapsedTime = C_MsCounter_Create()
  C_MsCounter_Reset(state.ElapsedTime)
  
  state.ShouldRotate = false
  
  state.LastSim60thSecondCount = 0
end

function MainApp_HandleEvent(state, e)
  local type = e.type
  if (type == 0x300) then -- SDL_KEYDOWN
    local keySym = e.keySym
    
    -- record that this key is now pressed down
    --state.KeyStates[keySym] = true
    --state.KeysWhenPressed[keySym] = C_MsCounter_GetCount(state.ElapsedTime)

    if (keySym == 27) then -- SDLK_ESCAPE
      C_Exit(0)
    elseif (keySym == 32) then -- SDLK_SPACE
      state.ShouldRotate = not state.ShouldRotate
    elseif (keySym == 97) then -- SDLK_a
      C_NonFatalError("user-initiated non-fatal error")
    elseif (keySym == 1073741905) then -- SDLK_DOWN
      -- start counting anew
      state.FloorZOffsetDirection = "closer"
      -- todo: do a better job gracefully handling multiple keystrokes and "both keys held down"
    elseif (keySym == 1073741906) then -- SDLK_UP
      -- start counting anew
      state.FloorZOffsetDirection = "farther"
      -- todo: do a better job gracefully handling multiple keystrokes and "both keys held down"
    end
  elseif type == 0x301 then -- SDL_KEYUP
    local keySym = e.keySym
    if (keySym == 1073741905) then -- SDLK_DOWN
      -- stop counting
      state.FloorZOffsetDirection = nil
      -- todo: do a better job gracefully handling multiple keystrokes and "both keys held down"
    elseif (keySym == 1073741906) then -- SDLK_UP
      -- stop counting
      state.FloorZOffsetDirection = nil      
      -- todo: do a better job gracefully handling multiple keystrokes and "both keys held down"
    end
    
    -- record that this key is now released
    --state.KeyStates[keySym] = nil
    --state.KeysWhenPressed[keySym] = nil    
  end
  -- TODO: window management, more input handling, others?
end

function MainApp_Process(state)
  if state.ShouldRotate then
    -- update our sense of time
    local msCount = C_MsCounter_Update(state.ElapsedTime)
    
    -- produce a full rotation every 2 seconds
    state.CurrentAngle = (msCount % 2000) * 360 / 2000
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

function MainApp_Draw(state)
  -- TODO: more engine, less hacks
  return state.CurrentAngle
end
