//
//  framework_sdl.cpp
//  archery
//
//  Created by exe on 9/1/13.
//
//

#include "framework_sdl.h"

#ifdef ERI_DESKTOP

#include "pch.h"

#include "root.h"
#include "renderer.h"
#include "input_mgr.h"

static ERI::InputKeyCode TranslateKeyCode(int event_key_code)
{
	ERI::InputKeyCode code = ERI::KEY_NONE;
  
  switch (event_key_code)
  {
    case SDLK_DELETE: code = ERI::KEY_DELETE; break;
    case SDLK_BACKSPACE: code = ERI::KEY_BACKSPACE; break;
    case SDLK_RETURN: code = ERI::KEY_RETUEN; break;
    case SDLK_ESCAPE: code = ERI::KEY_ESCAPE; break;
    case SDLK_TAB: code = ERI::KEY_TAB; break;
    case SDLK_LEFT: code = ERI::KEY_LEFT; break;
    case SDLK_RIGHT: code = ERI::KEY_RIGHT; break;
    case SDLK_DOWN: code = ERI::KEY_DOWN; break;
    case SDLK_UP: code = ERI::KEY_UP; break;
    case SDLK_w: code = ERI::KEY_W; break;
    case SDLK_s: code = ERI::KEY_S; break;
    case SDLK_a: code = ERI::KEY_A; break;
    case SDLK_d: code = ERI::KEY_D; break;
    case SDLK_SPACE: code = ERI::KEY_SPACE; break;
    case SDLK_q: code = ERI::KEY_Q; break;
    case SDLK_e: code = ERI::KEY_E; break;
    case SDLK_r: code = ERI::KEY_R; break;
    case SDLK_p: code = ERI::KEY_P; break;
    case SDLK_l: code = ERI::KEY_L; break;
    case SDLK_n: code = ERI::KEY_N; break;
    case SDLK_o: code = ERI::KEY_O; break;
    case SDLK_1: code = ERI::KEY_1; break;
    case SDLK_2: code = ERI::KEY_2; break;
  }
  
	return code;
}

static unsigned int GetFunctionKeyStatus()
{
	unsigned int status = 0;
  
  const Uint8* state = SDL_GetKeyboardState(NULL);

  if (state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT])
    status |= ERI::FUNC_SHIFT;
  if (state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL])
    status |= ERI::FUNC_CTRL;
  if (state[SDL_SCANCODE_LALT] || state[SDL_SCANCODE_RALT])
    status |= ERI::FUNC_ALT;
  if (state[SDL_SCANCODE_LGUI] || state[SDL_SCANCODE_RGUI])
    status |= ERI::FUNC_CMD;
	
	return status;
}

//static const char* ControllerAxisName(int axis)
//{
//  switch (axis)
//  {
//#define AXIS_CASE(ax) case SDL_CONTROLLER_AXIS_##ax: return #ax
//      AXIS_CASE(INVALID);
//      AXIS_CASE(LEFTX);
//      AXIS_CASE(LEFTY);
//      AXIS_CASE(RIGHTX);
//      AXIS_CASE(RIGHTY);
//      AXIS_CASE(TRIGGERLEFT);
//      AXIS_CASE(TRIGGERRIGHT);
//#undef AXIS_CASE
//    default: return "???";
//  }
//}

static const char* ControllerButtonName(int button)
{
  switch (button)
  {
#define BUTTON_CASE(btn) case SDL_CONTROLLER_BUTTON_##btn: return #btn
      BUTTON_CASE(INVALID);
      BUTTON_CASE(A);
      BUTTON_CASE(B);
      BUTTON_CASE(X);
      BUTTON_CASE(Y);
      BUTTON_CASE(BACK);
      BUTTON_CASE(GUIDE);
      BUTTON_CASE(START);
      BUTTON_CASE(LEFTSTICK);
      BUTTON_CASE(RIGHTSTICK);
      BUTTON_CASE(LEFTSHOULDER);
      BUTTON_CASE(RIGHTSHOULDER);
      BUTTON_CASE(DPAD_UP);
      BUTTON_CASE(DPAD_DOWN);
      BUTTON_CASE(DPAD_LEFT);
      BUTTON_CASE(DPAD_RIGHT);
#undef BUTTON_CASE
    default: return "???";
  }
}

static ERI::JoystickCode TranslateButtonJoystickCode(int event_joystick_code)
{
  ERI::JoystickCode code = ERI::JOYSTICK_NONE;
  
  switch (event_joystick_code)
  {
    case SDL_CONTROLLER_BUTTON_A: code = ERI::JOYSTICK_A; break;
    case SDL_CONTROLLER_BUTTON_B: code = ERI::JOYSTICK_B; break;
    case SDL_CONTROLLER_BUTTON_X: code = ERI::JOYSTICK_X; break;
    case SDL_CONTROLLER_BUTTON_Y: code = ERI::JOYSTICK_Y; break;
    case SDL_CONTROLLER_BUTTON_BACK: code = ERI::JOYSTICK_BACK; break;
    case SDL_CONTROLLER_BUTTON_GUIDE: break;
    case SDL_CONTROLLER_BUTTON_START: code = ERI::JOYSTICK_START; break;
    case SDL_CONTROLLER_BUTTON_LEFTSTICK: code = ERI::JOYSTICK_THUMBL; break;
    case SDL_CONTROLLER_BUTTON_RIGHTSTICK: code = ERI::JOYSTICK_THUMBR; break;
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: code = ERI::JOYSTICK_L1; break;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: code = ERI::JOYSTICK_R1; break;
    case SDL_CONTROLLER_BUTTON_DPAD_UP: code = ERI::JOYSTICK_DPAD_UP; break;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN: code = ERI::JOYSTICK_DPAD_DOWN; break;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT: code = ERI::JOYSTICK_DPAD_LEFT; break;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: code = ERI::JOYSTICK_DPAD_RIGHT; break;
      
    default:
      break;
  }
  
  return code;
}

Framework::Framework(int window_width, int window_height, const char* title /*= NULL*/, unsigned int flags /*= 0*/)
  : window_(NULL),
  context_(NULL),
  window_width_(window_width),
  window_height_(window_height),
  is_running_(false),
  is_visible_(false),
  is_fullscreen_(false),
  fullscreen_type_(SDL_WINDOW_FULLSCREEN_DESKTOP)
{
  ASSERT(window_width_ > 0 && window_height_ > 0);
  ASSERT((flags & (FULL_SCREEN | FULL_SCREEN_DESKTOP)) != (FULL_SCREEN | FULL_SCREEN_DESKTOP));
  
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER ) < 0)
  {
    ASSERT2(0, "Couldn't initialize SDL: %s", SDL_GetError());
  }
  
  memset(game_controllers_, 0, sizeof(game_controllers_));
  
  Uint32 sdl_flags = 0;
  if (flags & FULL_SCREEN)
  {
    fullscreen_type_ = SDL_WINDOW_FULLSCREEN;
    sdl_flags |= fullscreen_type_;
    is_fullscreen_ = true;
  }
  if (flags & FULL_SCREEN_DESKTOP)
  {
    sdl_flags |= fullscreen_type_;
    is_fullscreen_ = true;
  }
  if (flags & RESIZABLE) sdl_flags |= SDL_WINDOW_RESIZABLE;
  
  window_ = SDL_CreateWindow(title ? title : "sdl",
                             SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                             window_width, window_height,
                             sdl_flags | SDL_WINDOW_OPENGL);

  ASSERT2(window_, "Could not create window: %s", SDL_GetError());
  
  context_ = SDL_GL_CreateContext(window_);
  
  ASSERT2(context_, "Could not create context: %s", SDL_GetError());
  
  SDL_GetWindowSize(window_, &current_width_, &current_height_);

#if ERI_PLATFORM == ERI_PLATFORM_WIN || ERI_PLATFORM == ERI_PLATFORM_LINUX
  GLenum err = glewInit();
	ASSERT2(GLEW_OK == err, "Error: %s", glewGetErrorString(err));
	LOGI("Status: Using GLEW %s", glewGetString(GLEW_VERSION));
#endif
  
  ERI::Root::Ins().Init();
}

Framework::~Framework()
{
  ERI::Root::DestroyIns();
  
  for (int i = 0; i < kGameControllerMax; ++i)
  {
    if (NULL != game_controllers_[i])
      SDL_GameControllerClose(game_controllers_[i]);
  }
  
  SDL_GL_DeleteContext(context_);
  SDL_DestroyWindow(window_);
  SDL_Quit();
}

void Framework::Run()
{
  ERI::Root::Ins().renderer()->Resize(current_width_, current_height_);
  is_running_ = true;
}

float Framework::PreUpdate()
{
  SDL_Event event;

  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
      case SDL_QUIT:
        is_running_ = false;
        break;
        
      case SDL_WINDOWEVENT:
        switch (event.window.event)
        {
          case SDL_WINDOWEVENT_RESIZED:
            LOGI("Window %d resized to %dx%d",
                    event.window.windowID, event.window.data1,
                    event.window.data2);
            current_width_ = event.window.data1;
            current_height_ = event.window.data2;
            ERI::Root::Ins().renderer()->Resize(current_width_, current_height_);
            break;
            
          case SDL_WINDOWEVENT_CLOSE:
            is_running_ = false;
            break;
            
          case SDL_WINDOWEVENT_SHOWN:
            is_visible_ = true;
            LOGI("Window %d shown", event.window.windowID);
            break;
          case SDL_WINDOWEVENT_HIDDEN:
            is_visible_ = false;
            LOGI("Window %d hidden", event.window.windowID);
            break;
          case SDL_WINDOWEVENT_EXPOSED:
            LOGI("Window %d exposed", event.window.windowID);
            break;
          case SDL_WINDOWEVENT_MINIMIZED:
            LOGI("Window %d minimized", event.window.windowID);
            break;
          case SDL_WINDOWEVENT_MAXIMIZED:
            LOGI("Window %d maximized", event.window.windowID);
            break;
          case SDL_WINDOWEVENT_RESTORED:
            LOGI("Window %d restored", event.window.windowID);
            break;
            
          default:
            break;
        }
        break;
        
      case SDL_MOUSEMOTION:
        {
//          LOGI("Mouse: button %d moved to %d,%d (%d,%d) in window %d",
//               event.button.button,
//               event.motion.x, event.motion.y,
//               event.motion.xrel, event.motion.yrel,
//               event.motion.windowID);

          ERI::InputEvent e(0, event.motion.x, current_height_ - event.motion.y);
          e.dx = event.motion.xrel;
          e.dy = -event.motion.yrel;
          switch (event.button.button)
          {
            case 0:
              ERI::Root::Ins().input_mgr()->OverMove(e);
              break;
              
            case 1:
              ERI::Root::Ins().input_mgr()->Move(e);
              break;
              
            default:
              break;
          }
        }
        break;
        
      case SDL_MOUSEWHEEL:
        {
//          LOGI("Mouse: wheel scrolled %d in x and %d in y in window %d",
//               event.wheel.x, event.wheel.y, event.wheel.windowID);
          
          ERI::InputEvent e;
          e.dx = event.wheel.x;
          e.dy = event.wheel.y;
          e.function_key_status = GetFunctionKeyStatus();

          ERI::Root::Ins().input_mgr()->Scroll(e);
        }
        break;
        
      case SDL_MOUSEBUTTONDOWN:
        {
//          LOGI("Mouse: button %d pressed at %d,%d in window %d",
//               event.button.button, event.button.x, event.button.y,
//               event.button.windowID);

          ERI::InputEvent e(0, event.button.x, current_height_ - event.button.y);
          e.function_key_status = GetFunctionKeyStatus();
          
          switch (event.button.button)
          {
            case 1:
              ERI::Root::Ins().input_mgr()->Press(e);
              mouse_down_x_[0] = e.x;
              mouse_down_y_[0] = e.y;
              break;
              
            case 3:
              ERI::Root::Ins().input_mgr()->RightPress(e);
              mouse_down_x_[1] = e.x;
              mouse_down_y_[1] = e.y;
              break;
              
            default:
              break;
          }
        }
        break;
        
      case SDL_MOUSEBUTTONUP:
        {
//          LOGI("Mouse: button %d released at %d,%d in window %d",
//               event.button.button, event.button.x, event.button.y,
//               event.button.windowID);

          ERI::InputEvent e(0, event.button.x, current_height_ - event.button.y);
          e.function_key_status = GetFunctionKeyStatus();
          
          switch (event.button.button)
          {
            case 1:
              ERI::Root::Ins().input_mgr()->Release(e);
              if (ERI::Abs(e.x - mouse_down_x_[0]) < 10 && ERI::Abs(e.y - mouse_down_y_[0]) < 10)
                ERI::Root::Ins().input_mgr()->Click(e);
              break;
              
            case 3:
              ERI::Root::Ins().input_mgr()->RightRelease(e);
              if (ERI::Abs(e.x - mouse_down_x_[1]) < 10 && ERI::Abs(e.y - mouse_down_y_[1]) < 10)
                ERI::Root::Ins().input_mgr()->RightClick(e);
              break;
              
            default:
              break;
          }
        }
        break;
        
      case SDL_KEYDOWN:
        {
//          LOGI("Keyboard: key pressed  in window %d: scancode 0x%08X = %s, keycode 0x%08X = %s",
//               event.key.windowID,
//               event.key.keysym.scancode,
//               SDL_GetScancodeName(event.key.keysym.scancode),
//               event.key.keysym.sym, SDL_GetKeyName(event.key.keysym.sym));
          
          if (event.key.keysym.sym == SDLK_RETURN && event.key.keysym.mod & KMOD_ALT)
          {
            ToggleFullscreen();
            break;
          }
          
          ERI::InputKeyEvent e;
          
          // TODO:
          // e.characters = [characters UTF8String];
          
          e.code = TranslateKeyCode(event.key.keysym.sym);
          e.function_key_status = GetFunctionKeyStatus();
          
          ERI::Root::Ins().input_mgr()->KeyDown(e);
        }
        break;
        
      case SDL_KEYUP:
        {
//          LOGI("Keyboard: key released in window %d: scancode 0x%08X = %s, keycode 0x%08X = %s",
//               event.key.windowID,
//               event.key.keysym.scancode,
//               SDL_GetScancodeName(event.key.keysym.scancode),
//               event.key.keysym.sym, SDL_GetKeyName(event.key.keysym.sym));
          
          ERI::InputKeyEvent e;
          
          // TODO:
          //e.characters = [characters UTF8String];
          
          e.code = TranslateKeyCode(event.key.keysym.sym);
          e.function_key_status = GetFunctionKeyStatus();
          
          ERI::Root::Ins().input_mgr()->KeyUp(e);
        }
        break;
        
      case SDL_TEXTINPUT:
        {
          LOGI("Keyboard: text input \"%s\" in window %d",
               event.text.text, event.text.windowID);
          
          ERI::InputKeyEvent e;
          e.characters = event.text.text;
          
          ERI::Root::Ins().input_mgr()->Char(e);
        }
        break;
        
      case SDL_CONTROLLERAXISMOTION:
        {
//          LOGI("Controller %d axis %d ('%s') value: %d",
//               event.caxis.which,
//               event.caxis.axis,
//               ControllerAxisName(event.caxis.axis),
//               event.caxis.value);
          
          switch (event.caxis.axis)
          {
            case SDL_CONTROLLER_AXIS_LEFTX:
              axis_left_.x = event.caxis.value / 32768.f;
              ERI::Root::Ins().input_mgr()->JoystickAxis(ERI::JOYSTICK_AXISL, axis_left_.x, axis_left_.y);
              break;
            case SDL_CONTROLLER_AXIS_LEFTY:
              axis_left_.y = -event.caxis.value / 32768.f;
              ERI::Root::Ins().input_mgr()->JoystickAxis(ERI::JOYSTICK_AXISL, axis_left_.x, axis_left_.y);
              break;
            case SDL_CONTROLLER_AXIS_RIGHTX:
              axis_right_.x = event.caxis.value / 32768.f;
              ERI::Root::Ins().input_mgr()->JoystickAxis(ERI::JOYSTICK_AXISR, axis_right_.x, axis_right_.y);
              break;
            case SDL_CONTROLLER_AXIS_RIGHTY:
              axis_right_.y = -event.caxis.value / 32768.f;
              ERI::Root::Ins().input_mgr()->JoystickAxis(ERI::JOYSTICK_AXISR, axis_right_.x, axis_right_.y);
              break;
            case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
              ERI::Root::Ins().input_mgr()->JoystickAxis(ERI::JOYSTICK_L2, event.caxis.value / 32768.f, 0.f);
              break;
            case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
              ERI::Root::Ins().input_mgr()->JoystickAxis(ERI::JOYSTICK_R2, event.caxis.value / 32768.f, 0.f);
              break;
              
            default:
              break;
          }
        }
        break;
        
      case SDL_CONTROLLERBUTTONDOWN:
        {
          LOGI("Controller %d button %d ('%s') down",
               event.cbutton.which, event.cbutton.button,
               ControllerButtonName(event.cbutton.button));
          
          ERI::Root::Ins().input_mgr()->JoystickDown(TranslateButtonJoystickCode(event.cbutton.button));
        }
        break;
        
      case SDL_CONTROLLERBUTTONUP:
        {
          LOGI("Controller %d button %d ('%s') up",
               event.cbutton.which, event.cbutton.button,
               ControllerButtonName(event.cbutton.button));

          ERI::Root::Ins().input_mgr()->JoystickUp(TranslateButtonJoystickCode(event.cbutton.button));
        }
        break;
        
      case SDL_CONTROLLERDEVICEADDED:
        {
          LOGI("controller device added %d", event.cdevice.which);

          SDL_GameController* game_controller = SDL_GameControllerOpen(event.cdevice.which);
          if (game_controller)
          {
            AddGameController(game_controller);
          }
          else
          {
            LOGW("Couldn't open game controller %d: %s", event.cdevice.which, SDL_GetError());
          }
        }
        break;

      case SDL_CONTROLLERDEVICEREMOVED:
        {
          LOGI("controller device removed %d", event.cdevice.which);
          
          for (int i = 0; i < kGameControllerMax; ++i)
          {
            if (NULL != game_controllers_[i])
            {
              if (SDL_GameControllerGetAttached(game_controllers_[i]))
              {
                LOGI("game controller [%d] attached", i);
              }
              else
              {
                LOGI("game controller [%d] not attached, close it", i);
                
                SDL_GameControllerClose(game_controllers_[i]);
                game_controllers_[i] = NULL;
              }
            }
          }
        }
        break;

      default:
        break;
    }
  }
  
  // calculate delta time
  int now_time = SDL_GetTicks();
  static int last_time = now_time;
  static float delta_time = 0.f;
  
  delta_time = (now_time - last_time) * 0.001f;
  last_time = now_time;
  
  return delta_time;
}

void Framework::PostUpdate()
{
  ERI::Root::Ins().Update();
  
  SDL_GL_SwapWindow(window_); // Swap the window/buffer to display the result.
  SDL_Delay(10);              // Pause briefly before moving on to the next cycle.
}

void Framework::Stop()
{
  is_running_ = false;
}

void Framework::ToggleFullscreen()
{
  if (window_)
  {
    is_fullscreen_ = !is_fullscreen_;
    SDL_SetWindowFullscreen(window_, is_fullscreen_ ? fullscreen_type_ : SDL_FALSE);

    if (!is_fullscreen_)
    {
      SDL_SetWindowSize(window_, window_width_, window_height_);
      SDL_GetWindowSize(window_, &current_width_, &current_height_);
      ERI::Root::Ins().renderer()->Resize(current_width_, current_height_);
    }
    else
    {
      SDL_GetWindowSize(window_, &current_width_, &current_height_);
    }
  }
}

void Framework::AddGameController(SDL_GameController* game_controller)
{
  ASSERT(game_controller);
  
  int empty_idx = -1;
  for (int i = 0; i < kGameControllerMax; ++i)
  {
    if (-1 == empty_idx && NULL == game_controllers_[i])
    {
      empty_idx = i;
    }
    else
    {
      ASSERT(game_controller != game_controllers_[i]);
    }
  }

  if (-1 == empty_idx)
  {
    LOGW("no empty game controller slots! max %d", kGameControllerMax);
  }
  else
  {
    game_controllers_[empty_idx] = game_controller;
    LOGI("add game controller %s to slots[%d]!", SDL_GameControllerName(game_controller), empty_idx);
  }
}

#endif // ERI_DESKTOP
