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

class SurfaceFactoryWayland;
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
  virtual ~WaylandDisplay();

  static WaylandDisplay* GetDisplay();

  // Returns a pointer to the wl_display.
  wl_display* display() const { return display_; }

  wl_registry* registry() const { return registry_; }

  // Returns a list of the registered screens.
  std::list<WaylandScreen*> GetScreenList() const;
  WaylandScreen* PrimaryScreen() const { return primary_screen_ ; }

  wl_shell* shell() const { return shell_; }

  wl_shm* shm() const { return shm_; }

  void AddWindow(WaylandWindow* window);

  void RemoveWindow(WaylandWindow* window);

  bool IsWindow(WaylandWindow* window);

  void AddTask(WaylandTask* task);

  // Ensures display is flushed when
  // FlushTasks is called. This is only for
  // convenience.
  void addPendingTask() { handle_flush_ = true; }

  // Returns true if any pending tasks have been handled
  // otherwise returns false.
  bool ProcessTasks();

  // The call has no effect unless there are any pending
  // WaylandTasks, otherwise similar to Flush.
  void FlushTasks();

  // Handle any pending Wayland tasks and send
  // all buffered data on client side to server.
  void Flush();

  // Handle all pending events in queue.
  // Forces a round trip to server.
  int SyncDisplay();

  InputMethod* GetInputMethod() const;

  void SetSerial(uint32_t serial) { serial_ = serial; }

  uint32_t GetSerial() const { return serial_; }

  wl_compositor* GetCompositor() { return compositor_; }

  // callback.
  static void SyncCallback(void *data, struct wl_callback *callback, uint32_t serial);

 private:
  WaylandDisplay(char* name);
  void terminate();
  // Attempt to create a connection to the display. If it fails this returns
  // NULL
  static WaylandDisplay* Connect(char* name = NULL);
  static void DestroyDisplay();
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
  wl_registry* registry_;
  wl_compositor* compositor_;
  wl_shell* shell_;
  wl_shm* shm_;
  struct wl_event_queue* queue_;
  WaylandScreen* primary_screen_;

  std::list<WaylandScreen*> screen_list_;
  std::list<WaylandInputDevice*> input_list_;
  std::list<WaylandTask*> task_list_;
  std::list<WaylandWindow*> window_list_;
  WaylandInputMethodEventFilter *input_method_filter_;

  uint32_t serial_;
  bool handle_flush_ :1;

  friend class SurfaceFactoryWayland;
  DISALLOW_COPY_AND_ASSIGN(WaylandDisplay);
};

}  // namespace ui

#endif  // OZONE_WAYLAND_DISPLAY_H_
