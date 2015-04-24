
function MainApp_Initialize(state)
  state.CurrentAngle = 0.0

  state.ElapsedTime = C_MsCounter_Create()
  C_MsCounter_Reset(state.ElapsedTime)
  
  state.ShouldRotate = false
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
      C_NonFatalError("user-initiated non-fatal error");
    end
  end
  -- TODO: window management, more input handling, others?
end

function MainApp_Process(state)
  if state.ShouldRotate then
    -- update our sense of time
    local msCount = C_MsCounter_Update(state.ElapsedTime);
    
    -- produce a full rotation every 2 seconds
    state.CurrentAngle = (msCount % 2000) * 360 / 2000;
  else
    -- maintain a frozen sense of time
    C_MsCounter_ResetToCurrentCount(state.ElapsedTime);
  end;
end

function MainApp_Draw(state)
  -- TODO: more engine, less hacks
  return state.CurrentAngle;
end
