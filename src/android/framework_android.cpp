//
//  framework_android.cpp
//  eri
//
//  Created by exe on 8/5/15.
//

#include "framework_android.h"

#include <android/sensor.h>

#include "root.h"
#include "renderer.h"
#include "input_mgr.h"

// -----------------------------------------------------------------------------
// need implement this to fix build, why?

#include "rapidxml.hpp"

void rapidxml::parse_error_handler(const char *what, void *where)
{
  LOGW("Parse error: %s %p", what, where);
}

// -----------------------------------------------------------------------------

android_app* g_android_app = NULL;

// -----------------------------------------------------------------------------

struct SharedAppData
{
  ASensorManager* sensorManager;
  const ASensor* accelerometerSensor;
  ASensorEventQueue* sensorEventQueue;

  EGLDisplay display;
  EGLConfig config;
  EGLSurface surface;
  EGLContext context;

  int32_t width;
  int32_t height;
};

static SharedAppData s_app_data;

// -----------------------------------------------------------------------------
// Process the next main command.

void HandleAppCmd(android_app* app, int32_t cmd)
{
  Framework* fw = (Framework*)app->userData;

  switch (cmd)
  {
    case APP_CMD_SAVE_STATE:
      LOGI("APP_CMD_SAVE_STATE");
      break;

    case APP_CMD_INIT_WINDOW:
      LOGI("APP_CMD_INIT_WINDOW");
      // The window is being shown, get it ready.
      if (app->window)
        fw->InitDisplay();
      break;

    case APP_CMD_TERM_WINDOW:
      LOGI("APP_CMD_TERM_WINDOW");
      // The window is being hidden or closed, clean it up.
      fw->LostFocus();
      fw->TerminateDisplay();
      break;

    case APP_CMD_GAINED_FOCUS:
      LOGI("APP_CMD_GAINED_FOCUS");
      fw->GainFocus();
      break;

    case APP_CMD_LOST_FOCUS:
      LOGI("APP_CMD_LOST_FOCUS");
      fw->LostFocus();
      break;

    case APP_CMD_START:
      LOGI("APP_CMD_START");
      break;

    case APP_CMD_RESUME:
      LOGI("APP_CMD_RESUME");
      break;

    case APP_CMD_PAUSE:
      LOGI("APP_CMD_PAUSE");
      break;

    case APP_CMD_STOP:
      LOGI("APP_CMD_STOP");
      break;

    case APP_CMD_DESTROY:
      LOGI("APP_CMD_DESTROY");
      break;

    case APP_CMD_CONFIG_CHANGED:
      LOGI("APP_CMD_CONFIG_CHANGED");
      break;

    case APP_CMD_LOW_MEMORY:
      LOGI("APP_CMD_LOW_MEMORY");
      break;
  }
}

// -----------------------------------------------------------------------------
// input event handle

struct MotionPressState
{
  MotionPressState() : src(-1) {}

  int32_t src;
  float x, y;
};

#define STATE_RECORD_NUM 32
static MotionPressState motion_press_states[STATE_RECORD_NUM];

static void AddMotionPress(int32_t src, float x, float y)
{
  if (src < 0)
  {
    LOGW("error! motion src == 0");
    return;
  }

  int empty_slot = -1;
  for (int i = 0; i < STATE_RECORD_NUM; ++i)
  {
    if (motion_press_states[i].src == src)
    {
      motion_press_states[i].x = x;
      motion_press_states[i].y = y;
      return;
    }

    if (-1 == empty_slot && motion_press_states[i].src < 0)
      empty_slot = i;
  }

  if (-1 == empty_slot)
  {
    LOGW("error! motion state record full!");
    return;
  }

  motion_press_states[empty_slot].src = src;
  motion_press_states[empty_slot].x = x;
  motion_press_states[empty_slot].y = y;
}

static void ClearMotionPress(int32_t src)
{
  for (int i = 0; i < STATE_RECORD_NUM; ++i)
  {
    if (motion_press_states[i].src == src)
    {
      motion_press_states[i].src = -1;
      return;
    }
  }
}

static bool IsMotionClick(int32_t src, float x, float y)
{
  for (int i = 0; i < STATE_RECORD_NUM; ++i)
  {
    if (motion_press_states[i].src == src)
    {
      motion_press_states[i].src = -1;

      float diff_x = x - motion_press_states[i].x;
      float diff_y = y - motion_press_states[i].y;

      if ((diff_x * diff_x + diff_y * diff_y) < 25.f)
        return true;

      return false;
    }
  }  
}

static void ConvertPosByViewOrientation(float& x, float& y)
{
  switch (ERI::Root::Ins().renderer()->view_orientation())
  {
    case ERI::PORTRAIT_HOME_BOTTOM:
      y = s_app_data.height - y;
      break;

    case ERI::PORTRAIT_HOME_TOP:
      x = s_app_data.width - x;
      break;

    case ERI::LANDSCAPE_HOME_RIGHT:
      {
        float orig_x = x;
        x = y;
        y = orig_x;
      }
      break;

    case ERI::LANDSCAPE_HOME_LEFT:
      {
        float orig_x = x;
        x = s_app_data.height - y;
        y = s_app_data.width - orig_x;
      }
      break;

    default:
      LOGW("Error! Invalid Orientation Type %d", ERI::Root::Ins().renderer()->view_orientation());
      break;
  }
}

static void GetPointerInfo(AInputEvent* event, int32_t idx, int32_t& out_id, float& out_x, float& out_y)
{
  out_id = AMotionEvent_getPointerId(event, idx);
  out_x = AMotionEvent_getX(event, idx);
  out_y = AMotionEvent_getY(event, idx);
  ConvertPosByViewOrientation(out_x, out_y);
}

static int32_t HandleInputEvent(struct android_app* app, AInputEvent* event)
{
  Framework* fw = (Framework*)app->userData;

  if (!fw->IsReady())
    return 0;

  if (AINPUT_EVENT_TYPE_MOTION == AInputEvent_getType(event))
  {
    int32_t action = AMotionEvent_getAction(event);
    int32_t pointer_idx = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

    static bool in_multi_move = false;

    int32_t pointer_id;
    float x, y;

    switch (action & AMOTION_EVENT_ACTION_MASK)
    {
      case AMOTION_EVENT_ACTION_DOWN:
      case AMOTION_EVENT_ACTION_POINTER_DOWN:
        GetPointerInfo(event, pointer_idx, pointer_id, x, y);
        // LOGI("%d press x %f y %f", pointer_id, x, y);
        ERI::Root::Ins().input_mgr()->Press(ERI::InputEvent(pointer_id, x, y));
        AddMotionPress(pointer_id, x, y);
        break;

      case AMOTION_EVENT_ACTION_UP:
      case AMOTION_EVENT_ACTION_POINTER_UP:
          {
            GetPointerInfo(event, pointer_idx, pointer_id, x, y);
            // LOGI("%d release x %f y %f", pointer_id, x, y);

            ERI::InputEvent e(pointer_id, x, y);
            ERI::Root::Ins().input_mgr()->Release(e);
            if (IsMotionClick(pointer_id, x, y))
            {
              // LOGI("%d click x %f y %f", pointer_id, x, y);
              ERI::Root::Ins().input_mgr()->Click(e);
            }

            if (AMotionEvent_getPointerCount(event) <= 2)
              in_multi_move = false;
          }
          break;

        case AMOTION_EVENT_ACTION_MOVE:
          {
            int count = AMotionEvent_getPointerCount(event);
            if (count > 1)
            {
              static ERI::InputEvent events[16];

              for (int i = 0; i < count; ++i)
              {
                GetPointerInfo(event, i, pointer_id, x, y);
                // LOGI("%d move x %f y %f", pointer_id, x, y);

                events[i].uid = pointer_id;
                events[i].x = x;
                events[i].y = y;
              }

              ERI::Root::Ins().input_mgr()->MultiMove(events, count, !in_multi_move);

              in_multi_move = true;
            }
            else
            {
              GetPointerInfo(event, 0, pointer_id, x, y);
              // LOGI("%d move x %f y %f", pointer_id, x, y);
              ERI::Root::Ins().input_mgr()->Move(ERI::InputEvent(pointer_id, x, y));

              in_multi_move = false;
            }
          }
          break;

        case AMOTION_EVENT_ACTION_CANCEL:
          pointer_id = AMotionEvent_getPointerId(event, pointer_idx);
          ClearMotionPress(pointer_id);
          break;

        case AMOTION_EVENT_ACTION_OUTSIDE:
          break;
    }

    return 1;
  }
  else if (AINPUT_EVENT_TYPE_KEY == AInputEvent_getType(event))
  {
    int ret = 0;

    int32_t key_code = AKeyEvent_getKeyCode(event);
    // LOGI("keycode %d", key_code);

    switch (AKeyEvent_getAction(event))
    {
      case AKEY_EVENT_ACTION_DOWN:
        // LOGI("AKEY_EVENT_ACTION_DOWN");
        if (AKEYCODE_BACK == key_code)
        {
          ERI::Root::Ins().input_mgr()->KeyDown(ERI::InputKeyEvent(ERI::KEY_APP_BACK));
          ret = 1;
        }
        break;

      case AKEY_EVENT_ACTION_UP:
        // LOGI("AKEY_EVENT_ACTION_UP");
        if (AKEYCODE_BACK == key_code)
        {
          ERI::Root::Ins().input_mgr()->KeyUp(ERI::InputKeyEvent(ERI::KEY_APP_BACK));
          ret = 1;
        }
        break;

      case AKEY_EVENT_ACTION_MULTIPLE:
        // LOGI("AKEY_EVENT_ACTION_MULTIPLE");
        break;
    }

    return ret;
  }

  return 0;
}

// -----------------------------------------------------------------------------
// Framework

Framework::Framework(android_app* state, FrameworkConfig* config /*= NULL*/)
  : state_(state),
  has_focus_(false),
  prev_ns_(0),
  display_rotation_(0),
  refresh_display_rotation_timer_(0.f),
  is_stopping_(false),
  is_stopped_(false),
  create_app_callback_(NULL),
  destroy_app_callback_(NULL)
{
  ASSERT(state_);

  g_android_app = state_;

  memset(&s_app_data, 0, sizeof(s_app_data));
  state_->userData = this;
  state_->onAppCmd = HandleAppCmd;
  state_->onInputEvent = HandleInputEvent;

  InitSensor();

  //

  if (config)
    config_ = *config;
}

Framework::~Framework()
{
}

void Framework::SetAppCallback(FrameworkCallback create_app_callback, 
                               FrameworkCallback destroy_app_callback)
{
  create_app_callback_ = create_app_callback;
  destroy_app_callback_ = destroy_app_callback;
}

float Framework::PreUpdate()
{
  int id;
  int events;
  struct android_poll_source* source;

  // If not animating, we will block forever waiting for events.
  // If animating, we loop until all events are read, then continue
  // to draw the next frame of animation.
  while ((id = ALooper_pollAll(has_focus_ ? 0 : -1, NULL, &events, (void**)&source)) >= 0)
  {
    // Process this event.
    if (source)
      source->process(state_, source);

    ProcessSensor(id);

    // Check if we are exiting.
    if (0 != state_->destroyRequested)
    {
      LostFocus();
      TerminateDisplay();
      is_stopped_ = true;
      return 0.f;
    }
  }

  if (!IsReady())
    return 0.f;

  // calculate delta time

  timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  uint64_t now_ns = now.tv_sec * 1000000000ull + now.tv_nsec;

  if (prev_ns_ == 0)
      prev_ns_ = now_ns;

  double delta_time = double(now_ns - prev_ns_) * 0.000000001f;
  prev_ns_ = now_ns;

  // refresh display rotation

  refresh_display_rotation_timer_ += delta_time;
  if (refresh_display_rotation_timer_ > 1.f)
  {
    RefreshDisplayRotation();
    refresh_display_rotation_timer_ = 0.f;
  }

  return delta_time;
}

bool Framework::IsReady()
{
  return has_focus_ && (EGL_NO_DISPLAY != s_app_data.display);
}

void Framework::PostUpdate()
{
  if (!IsReady())
    return;

  ERI::Root::Ins().Update();

  bool b = eglSwapBuffers(s_app_data.display, s_app_data.surface);
  if (!b)
  {
    EGLint err = eglGetError();

    if (EGL_BAD_SURFACE == err)
    {
      LOGW("EGL_BAD_SURFACE");
    }
    else if (EGL_CONTEXT_LOST == err)
    {
      LOGW("EGL_CONTEXT_LOST");
    }
    else if (EGL_BAD_CONTEXT == err)
    {
      LOGW("EGL_BAD_CONTEXT");
    }
    else
    {
      LOGW("eglSwapBuffers failed, error: %d", err);
    }

    // TODO: terminate?
  }
}

void Framework::RequestStop()
{
  if (is_stopping_ || is_stopped_)
    return;

  is_stopping_ = true;

  ANativeActivity_finish(state_->activity);
}

void Framework::InitDisplay()
{
  if (InitSurface())
    InitContext();

  //

  ERI::Root::Ins().Init(config_.use_depth_buffer);

  if (create_app_callback_)
    (*create_app_callback_)();

  ERI::Root::Ins().renderer()->Resize(s_app_data.width, s_app_data.height);

  LOGI("framework display inited: %d x %d", s_app_data.width, s_app_data.height);
}

bool Framework::InitSurface()
{
  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  eglInitialize(display, 0, 0);

  /*
   * Here specify the attributes of the desired configuration.
   * Below, we select an EGLConfig with at least 8 bits per color
   * component compatible with on-screen windows
   */
  const EGLint attribs[] = {
#ifdef ERI_RENDERER_ES2
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
#endif
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_DEPTH_SIZE, config_.use_depth_buffer ? 16 : 0,
    EGL_NONE
  };

  /* Here, the application chooses the configuration it desires. In this
   * sample, we have a very simplified selection process, where we pick
   * the first EGLConfig that matches our criteria */
  EGLint num_configs;
  EGLConfig config;
  eglChooseConfig(display, attribs, &config, 1, &num_configs);

  if (!num_configs)
  {
    LOGW("Unable to retrieve EGL config");
    return false;
  }

  /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
   * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
   * As soon as we picked a EGLConfig, we can safely reconfigure the
   * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
  EGLint format;
  eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
  ANativeWindow_setBuffersGeometry(state_->window, 0, 0, format);

  EGLSurface surface;
  EGLint w, h;

  surface = eglCreateWindowSurface(display, config, state_->window, NULL);
  eglQuerySurface(display, surface, EGL_WIDTH, &w);
  eglQuerySurface(display, surface, EGL_HEIGHT, &h);

  s_app_data.display = display;
  s_app_data.config = config;
  s_app_data.surface = surface;
  s_app_data.width = w;
  s_app_data.height = h;

  return true;
}

bool Framework::InitContext()
{
  const EGLint context_attribs[] = {
#ifdef ERI_RENDERER_ES2
    EGL_CONTEXT_CLIENT_VERSION, 2,
#endif
    EGL_NONE
  };

  EGLContext context = eglCreateContext(s_app_data.display, s_app_data.config, NULL, context_attribs);
  if (EGL_FALSE == eglMakeCurrent(s_app_data.display, s_app_data.surface, s_app_data.surface, context))
  {
    LOGW("Unable to eglMakeCurrent");
    return false;
  }

  s_app_data.context = context;

  return true;
}

void Framework::TerminateDisplay()
{
  if (EGL_NO_DISPLAY != s_app_data.display)
  {
    if (destroy_app_callback_)
      (*destroy_app_callback_)();

    ERI::Root::DestroyIns();

    //

    eglMakeCurrent(s_app_data.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    if (EGL_NO_CONTEXT != s_app_data.context)
        eglDestroyContext(s_app_data.display, s_app_data.context);

    if (EGL_NO_SURFACE != s_app_data.surface)
        eglDestroySurface(s_app_data.display, s_app_data.surface);

    eglTerminate(s_app_data.display);
  }

  s_app_data.context = EGL_NO_CONTEXT;
  s_app_data.surface = EGL_NO_SURFACE;
  s_app_data.display = EGL_NO_DISPLAY;

  LOGI("framework display terminated");
}

void Framework::GainFocus()
{
  has_focus_ = true;

  prev_ns_ = 0;
  refresh_display_rotation_timer_ = 0.f;

  ResumeSensor();
}

void Framework::LostFocus()
{
  SuspendSensor();

  has_focus_ = false;
}

void Framework::InitSensor()
{
  s_app_data.sensorManager = ASensorManager_getInstance();
  s_app_data.accelerometerSensor =
    ASensorManager_getDefaultSensor(s_app_data.sensorManager, ASENSOR_TYPE_ACCELEROMETER);
  s_app_data.sensorEventQueue =
    ASensorManager_createEventQueue(s_app_data.sensorManager, state_->looper, LOOPER_ID_USER, NULL, NULL);
}

void Framework::ResumeSensor()
{
  if (s_app_data.accelerometerSensor)
  {
    ASensorEventQueue_enableSensor(s_app_data.sensorEventQueue, s_app_data.accelerometerSensor);
    // We'd like to get 60 events per second (in us).
    ASensorEventQueue_setEventRate(s_app_data.sensorEventQueue, s_app_data.accelerometerSensor, (1000L/10)*1000);
  }
}

void Framework::SuspendSensor()
{
  if (s_app_data.accelerometerSensor)
    ASensorEventQueue_disableSensor(s_app_data.sensorEventQueue, s_app_data.accelerometerSensor);
}

void Framework::ProcessSensor(int id)
{
  if (NULL == s_app_data.accelerometerSensor)
    return;

  // If a sensor has data, process it now.
  if (LOOPER_ID_USER != id)
    return;

  ASensorEvent event;
  while (ASensorEventQueue_getEvents(s_app_data.sensorEventQueue, &event, 1) > 0)
  {
    if (!has_focus_)
      continue;

    // LOGI("accelerometer: x=%f y=%f z=%f",
    //         event.acceleration.x, event.acceleration.y,
    //         event.acceleration.z);

    ERI::Vector3 g(-event.acceleration.x, -event.acceleration.y, -event.acceleration.z);

    switch (display_rotation_)
    {
      case 1: // Surface.ROTATION_90
        {
          float orig_x = g.x;
          g.x = -g.y;
          g.y = orig_x;
        }
        break;

      case 2: // Surface.ROTATION_180
        {
          g.x = -g.x;
          g.y = -g.y;
        }
        break;

      case 3: // Surface.ROTATION_270
        {
          float orig_x = g.x;
          g.x = g.y;
          g.y = -orig_x;
        }
        break;
    }

    // LOGI("%f %f %f", g.x, g.y, g.z);

    ERI::Root::Ins().input_mgr()->Accelerate(g);
  }
}

void Framework::RefreshDisplayRotation()
{
  mana::JavaCaller caller;
  caller.Set("com/exe/eri", "GetDisplayRotate", "(Landroid/app/Activity;)I");
  int current_display_rotation = caller.env->CallStaticIntMethod(caller.user_class, caller.user_func, state_->activity->clazz);
  caller.End();

  if (display_rotation_ != current_display_rotation)
  {
    display_rotation_ = current_display_rotation;
    LOGI("display rotation %d", display_rotation_);
  }
}
