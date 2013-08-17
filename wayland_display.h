// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_DISPLAY_H_
#define OZONE_WAYLAND_DISPLAY_H_

#if !defined(__STDC_FORMAT_MACROS)
#define __STDC_FORMAT_MACROS
#endif

#include <list>
#include <stdint.h>
#include <wayland-client.h>

#include "ui/gfx/point.h"
#include "ui/gfx/rect.h"
#include "ozone/wayland_task.h"

namespace ui {

class WaylandInputDevice;
class WaylandScreen;
class WaylandWindow;
class WaylandInputMethodEventFilter;
class InputMethod;

// WaylandDisplay is a wrapper around wl_display. Once we get a valid
// wl_display, the Wayland server will send different events to register
// the Wayland compositor, shell, screens, input devices, ...
class WaylandDisplay {
 public:
  WaylandDisplay(char* name);
  virtual ~WaylandDisplay();

  // Attempt to create a connection to the display. If it fails this returns
  // NULL
  static WaylandDisplay* Connect(char* name);

  // Get the WaylandDisplay associated with the native Wayland display
  static WaylandDisplay* GetDisplay(wl_display* display);

  static WaylandDisplay* GetDisplay();

  static void DestroyDisplay();

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

  void AddWindow(WaylandWindow* window);

  void RemoveWindow(WaylandWindow* window);

  bool IsWindow(WaylandWindow* window);

  void AddTask(WaylandTask* task);

  // Returns true if any pending tasks have been handled otherwise
  // returns false.
  bool ProcessTasks();

  // Handles any pending Wayland tasks and sends
  // all buffered data on client side to the server.
  // The call has no effect if there are no pending
  // WaylandTasks.
  void FlushTasks();

  // Handles any pending Wayland tasks and sends
  // all buffered data on client side to the server.
  void Flush();

  void SetPointerImage(WaylandInputDevice* device, int pointer);

  InputMethod* GetInputMethod() const;

  void SetSerial(uint32_t serial) { serial_ = serial; }

  uint32_t GetSerial() const { return serial_; }

  wl_compositor* GetCompositor() { return compositor_; }

 private:
  void scheduleFlush() { handle_flush_ = true; }

  // This handler resolves all server events used in initialization. It also
  // handles input device registration, screen registration.
  static void DisplayHandleGlobal(
      void *data,
      struct wl_registry *registry,
      uint32_t name,
      const char *interface,
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

  uint32_t serial_;
  bool handle_flush_ :1;

  friend class WaylandWindow;
  DISALLOW_COPY_AND_ASSIGN(WaylandDisplay);
};

}  // namespace ui

#endif  // OZONE_WAYLAND_DISPLAY_H_
