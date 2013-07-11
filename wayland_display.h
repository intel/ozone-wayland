// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_DISPLAY_H_
#define OZONE_WAYLAND_DISPLAY_H_

#include <stdint.h>

#include <list>

#if !defined(__STDC_FORMAT_MACROS)
#define __STDC_FORMAT_MACROS
#endif

#include "ui/gfx/point.h"
#include "ui/gfx/rect.h"
#include <wayland-client.h>
#include "ozone/wayland_task.h"

#ifndef GLAPIENTRY
#define GLAPIENTRY
#endif

#ifndef APIENTRY
#define APIENTRY GLAPIENTRY
#endif

/* "P" suffix to be used for a pointer to a function */
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif

typedef unsigned int	GLenum;
typedef void* GLeglImageOES;
typedef void (APIENTRYP PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) (GLenum target, GLeglImageOES image);

struct wl_compositor;
struct wl_display;
struct wl_surface;
struct wl_shell;
struct wl_shm;
struct wl_data_device_manager;
struct wl_list;
struct wl_buffer;
struct wl_cursor_theme;
struct wl_cursor;

struct _cairo_surface;
typedef struct _cairo_surface cairo_surface_t;
struct _cairo_device;
typedef struct _cairo_device cairo_device_t;
struct _cairo_rectangle_int;
typedef struct _cairo_rectangle_int cairo_rectangle_int_t;

#include <EGL/eglplatform.h>
typedef unsigned int EGLBoolean;
typedef unsigned int EGLenum;
typedef void *EGLConfig;
typedef void *EGLContext;
typedef void *EGLDisplay;
typedef void *EGLSurface;
typedef void *EGLClientBuffer;

typedef void *EGLImageKHR;
typedef EGLImageKHR (EGLAPIENTRYP PFNEGLCREATEIMAGEKHRPROC) (EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLDESTROYIMAGEKHRPROC) (EGLDisplay dpy, EGLImageKHR image);

struct wl_compositor;
struct wl_display;
struct wl_shell;
struct wl_shm;
struct wl_surface;
struct wl_shell_surface;

namespace ui {

class WaylandBuffer;
class WaylandInputDevice;
class WaylandScreen;
class WaylandWindow;
class WaylandInputMethodEventFilter;
class InputMethod;

enum pointer_type {
  POINTER_BOTTOM_LEFT,
  POINTER_BOTTOM_RIGHT,
  POINTER_BOTTOM,
  POINTER_DRAGGING,
  POINTER_LEFT_PTR,
  POINTER_LEFT,
  POINTER_RIGHT,
  POINTER_TOP_LEFT,
  POINTER_TOP_RIGHT,
  POINTER_TOP,
  POINTER_IBEAM,
  POINTER_HAND1,
};

enum {
  POINTER_DEFAULT = 100,
  POINTER_UNSET
};

// WaylandDisplay is a wrapper around wl_display. Once we get a valid
// wl_display, the Wayland server will send different events to register
// the Wayland compositor, shell, screens, input devices, ...
class WaylandDisplay {
 public:
  // Attempt to create a connection to the display. If it fails this returns
  // NULL
  static WaylandDisplay* Connect(char* name);

  // Get the WaylandDisplay associated with the native Wayland display
  static WaylandDisplay* GetDisplay(wl_display* display);

  static WaylandDisplay* GetDisplay();
  static void DestroyDisplay();

  virtual ~WaylandDisplay();

  // Creates a wayland surface. This is used to create a window surface.
  // The returned pointer should be deleted by the caller.
  wl_surface* CreateSurface();

  // Returns a pointer to the wl_display.
  wl_display* display() const { return display_; }

  wl_registry* registry() const { return registry_; }

  // Returns a list of the registered screens.
  std::list<WaylandScreen*> GetScreenList() const;

  wl_shell* shell() const { return shell_; }

  wl_shm* shm() const { return shm_; }

  void DestroyEGLImage(EGLImageKHR image);

  void AddWindow(WaylandWindow* window);
  void RemoveWindow(WaylandWindow* window);
  bool IsWindow(WaylandWindow* window);

  void AddTask(WaylandTask* task);
  void ProcessTasks();

  cairo_surface_t* CreateSurfaceFromFile(const char *filename, gfx::Rect *rect);
  cairo_surface_t* CreateSurface(wl_surface *surface, gfx::Rect *rect, uint32_t flags);

  void AddWindowCallback(const wl_callback_listener *listener, WaylandWindow *window);

  void SetPointerImage(WaylandInputDevice* device, uint32_t time, int pointer);

  InputMethod* GetInputMethod() const;

  void SetSerial(uint32_t serial) { serial_ = serial; }
  uint32_t GetSerial() const { return serial_; }

 private:
  WaylandDisplay(char* name);
  void CreateCursors();
  void DestroyCursors();

  // This handler resolves all server events used in initialization. It also
  // handles input device registration, screen registration.
  static void DisplayHandleGlobal(void *data, struct wl_registry *registry,
                                  uint32_t name, const char *interface,
                                  uint32_t version);

  // WaylandDisplay manages the memory of all these pointers.
  wl_display* display_;
  wl_registry *registry_;
  wl_compositor* compositor_;
  wl_shell* shell_;
  wl_shm* shm_;

  std::list<WaylandScreen*> screen_list_;
  std::list<WaylandInputDevice*> input_list_;
  std::list<WaylandTask*> task_list_;
  std::list<WaylandWindow*> window_list_;
  WaylandInputMethodEventFilter *input_method_filter_;

  EGLDisplay dpy_;
  EGLConfig argb_config_;
  EGLContext argb_ctx_;
  cairo_device_t *argb_device_;
  uint32_t serial_;

  wl_cursor_theme *cursor_theme_;
  wl_cursor **cursors_;

  PFNGLEGLIMAGETARGETTEXTURE2DOESPROC image_target_texture_2d_;
  PFNEGLCREATEIMAGEKHRPROC create_image_;
  PFNEGLDESTROYIMAGEKHRPROC destroy_image_;

  friend class WaylandEGLImageSurfaceData;
  friend class WaylandEGLWindowSurfaceData;

  DISALLOW_COPY_AND_ASSIGN(WaylandDisplay);
};

}  // namespace ui

#endif  // OZONE_WAYLAND_DISPLAY_H_
