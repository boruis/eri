//
//  framework_android.h
//  eri
//
//  Created by exe on 8/5/15.
//

#ifndef ERI_FRAMEWORK_ANDROID_H
#define ERI_FRAMEWORK_ANDROID_H

#include "pch.h"

#include "android_helper.h"

typedef void (*FrameworkCallback)();

struct FrameworkConfig
{
  FrameworkConfig() :
    custom_scale(0.f),
    use_depth_buffer(true)
  {
  }
  
  float custom_scale;
  bool use_depth_buffer;
};

class Framework
{
public:
  Framework(android_app* state, FrameworkConfig* config = NULL);
  ~Framework();

  void SetAppCallback(FrameworkCallback create_app_callback, 
                      FrameworkCallback destroy_app_callback);

  float PreUpdate();
  bool IsReady();
  void PostUpdate();

  void RequestStop();
  inline bool is_stopped() { return is_stopped_; }

private:
  void InitDisplay();
  bool InitSurface();
  bool InitContext();
  void TerminateDisplay();

  void GainFocus();
  void LostFocus();

  void InitSensor();
  void ResumeSensor();
  void SuspendSensor();
  void ProcessSensor(int id);

  void RefreshDisplayRotation();

  android_app* state_;

  FrameworkConfig config_;

  bool has_focus_;
  uint64_t prev_ns_;

  int display_rotation_;
  float refresh_display_rotation_timer_;

  bool is_stopping_, is_stopped_;

  FrameworkCallback create_app_callback_;
  FrameworkCallback destroy_app_callback_;

  friend void HandleAppCmd(android_app* app, int32_t cmd);
};

#endif // ERI_FRAMEWORK_ANDROID_H
