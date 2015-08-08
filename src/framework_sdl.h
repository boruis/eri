//
//  framework_sdl.h
//  eri
//
//  Created by exe on 9/1/13.
//
//

#ifndef ERI_FRAMEWORK_SDL_H
#define ERI_FRAMEWORK_SDL_H

#include "pch.h"

#ifdef ERI_USE_SDL

// TODO: not sure why can't include SDL2 header in emscripten
//       disable SDL2 related funtions temporarilly
#if ERI_PLATFORM != ERI_PLATFORM_EMSCRIPTEN
#define ERI_USE_SDL_GAME_CONTROLLER
#define ERI_USE_SDL_THREAD
#define ERI_USE_SDL_FULLSCREEN
#endif

#include <cstddef>
#include <SDL.h>

#include "math_helper.h"

class Framework
{
public:
  enum Flag {
    FULL_SCREEN         = 0x01,
    FULL_SCREEN_DESKTOP = 0x02,
    RESIZABLE           = 0x04,
    NO_DEPTH_BUFFER     = 0x08
  };
  
  Framework(int window_width, int window_height, const char* title = NULL, unsigned int flags = 0);
  ~Framework();

  void Run();
  float PreUpdate();
  void PostUpdate();
  void Stop();
  
#ifdef ERI_USE_SDL_THREAD
  void* CreateThread(int (*thread_func)(void*), const char* name, void* data);
#endif
  
  void LogFPS(bool enable);
  
  void ToggleFullscreen();
  
  inline bool is_running() { return is_running_; }
  inline bool is_visible() { return is_visible_; }
  inline bool is_fullscreen() { return is_fullscreen_; }
  
private:
#ifdef ERI_USE_SDL_GAME_CONTROLLER
  void AddGameController(SDL_GameController* game_controller);
  
  static const int kGameControllerMax = 8;
  
  ERI::Vector2 axis_left_, axis_right_;
  SDL_GameController* game_controllers_[kGameControllerMax];
#endif
  
  SDL_Window* window_;
  SDL_GLContext context_;

  int window_width_, window_height_;
  int current_width_, current_height_;
  bool is_running_, is_visible_, is_fullscreen_;
  unsigned int fullscreen_type_;
  
  int mouse_down_x_[2];
  int mouse_down_y_[2];
  
  bool log_fps_;
  float frame_pass_time_;
  float frame_count_;
};

#endif // ERI_USE_SDL

#endif // ERI_FRAMEWORK_SDL_H
