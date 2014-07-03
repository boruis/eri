//
//  framework_sdl.h
//  archery
//
//  Created by exe on 9/1/13.
//
//

#ifndef __archery__framework_sdl__
#define __archery__framework_sdl__

#include "pch.h"

#ifdef ERI_DESKTOP

#include <cstddef>

#include "SDL.h"

#include "math_helper.h"

class Framework
{
public:
  enum Flag {
    FULL_SCREEN         = 0x01,
    FULL_SCREEN_DESKTOP = 0x02,
    RESIZABLE           = 0x04
  };
  
  Framework(int window_width, int window_height, const char* title = NULL, unsigned int flags = 0);
  ~Framework();

  void Run();
  float PreUpdate();
  void PostUpdate();
  void Stop();
  
  int GetTicksTime();
  void* CreateThread(int (*thread_func)(void*), const char* name, void* data);
  void Delay(int ms);
  
  void LogFPS(bool enable);
  
  void ToggleFullscreen();
  
  inline bool is_running() { return is_running_; }
  inline bool is_visible() { return is_visible_; }
  inline bool is_fullscreen() { return is_fullscreen_; }
  
  static const int kGameControllerMax = 8;

private:
  void AddGameController(SDL_GameController* game_controller);
  
  SDL_Window* window_;
  SDL_GLContext context_;

  int window_width_, window_height_;
  int current_width_, current_height_;
  bool is_running_, is_visible_, is_fullscreen_;
  unsigned int fullscreen_type_;
  
  int mouse_down_x_[2];
  int mouse_down_y_[2];
  
  ERI::Vector2 axis_left_, axis_right_;
  
  SDL_GameController* game_controllers_[kGameControllerMax];
  
  bool log_fps_;
  float frame_pass_time_;
  float frame_count_;
};

#endif // ERI_DESKTOP

#endif /* defined(__archery__framework_sdl__) */
