// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
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

namespace ui {

class SurfaceFactoryWayland;
class WaylandInputDevice;
class WaylandScreen;
class WaylandWindow;
class WaylandInputMethodEventFilter;
class InputMethod;
class WaylandDispatcher;

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

  void SetSerial(uint32_t serial) { serial_ = serial; }

  uint32_t GetSerial() const { return serial_; }

  wl_compositor* GetCompositor() { return compositor_; }
  WaylandDispatcher* Dispatcher() const { return dispatcher_; }

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
  WaylandScreen* primary_screen_;
  WaylandDispatcher* dispatcher_;

  std::list<WaylandScreen*> screen_list_;
  std::list<WaylandInputDevice*> input_list_;

  uint32_t serial_;

  friend class SurfaceFactoryWayland;
  DISALLOW_COPY_AND_ASSIGN(WaylandDisplay);
};

}  // namespace ui

#endif  // OZONE_WAYLAND_DISPLAY_H_
