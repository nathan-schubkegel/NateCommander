
function MainApp_Initialize(state)
  state.CurrentAngle = 0.0

  state.ElapsedTime = C_MsCounter_Create()
  C_MsCounter_Reset(state.ElapsedTime)
  
  state.ShouldRotate = false
  
  state.LastSim60thSecondCount = 0
end

function MainApp_HandleEvent(state, e)
  local type = e.type
  if (type == 0x300) then -- SDL_KEYDOWN
    local keySym = e.keySym
    if (keySym == 27) then -- SDLK_ESCAPE
      C_Exit(0)
    elseif (keySym == 32) then -- SDLK_SPACE
      state.ShouldRotate = not state.ShouldRotate
    elseif (keySym == 97) then -- SDLK_a
      C_NonFatalError("user-initiated non-fatal error")
    end
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
  
  -- also advance the simulator for every 1/60th of a second that goes by
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
      C_AdvanceGSIM()
    end
  end
end

function MainApp_Draw(state)
  -- TODO: more engine, less hacks
  return state.CurrentAngle
end
