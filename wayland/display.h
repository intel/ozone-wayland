// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_DISPLAY_H_
#define OZONE_WAYLAND_DISPLAY_H_

#if !defined(__STDC_FORMAT_MACROS)
#define __STDC_FORMAT_MACROS
#endif

#include "base/basictypes.h"
#include <list>
#include <wayland-client.h>

namespace ui {

class SurfaceFactoryWayland;
class WaylandInputDevice;
class WaylandScreen;
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
  std::list<WaylandScreen*> GetScreenList() const { return screen_list_; }
  WaylandScreen* PrimaryScreen() const { return primary_screen_ ; }

  // Returns a list of the registered input devices.
  std::list<WaylandInputDevice*> GetInputList() const { return input_list_; }
  WaylandInputDevice* PrimaryInputDevice() const { return primary_input_device_ ; }

  wl_shell* shell() const { return shell_; }

  wl_shm* shm() const { return shm_; }

  WaylandDispatcher* Dispatcher() const { return dispatcher_; }

  wl_compositor* GetCompositor() { return compositor_; }

 private:
  WaylandDisplay();
  void Connect();
  void Terminate();
  // Attempt to create a connection to the display. If it fails this returns
  // NULL
  static WaylandDisplay* ConnectDisplay();
  static void DestroyDisplay();

  // used internally to open a static connection.
  static WaylandDisplay* OpenStaticConnection();
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
  WaylandInputDevice* primary_input_device_;
  WaylandDispatcher* dispatcher_;

  std::list<WaylandScreen*> screen_list_;
  std::list<WaylandInputDevice*> input_list_;
  static WaylandDisplay* m_instance;

  friend class SurfaceFactoryWayland;
  DISALLOW_COPY_AND_ASSIGN(WaylandDisplay);
};

}  // namespace ui

#endif  // OZONE_WAYLAND_DISPLAY_H_
