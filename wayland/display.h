// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
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

namespace ozonewayland {

class WaylandInputDevice;
class WaylandScreen;
class OzoneDisplay;

// WaylandDisplay is a wrapper around wl_display. Once we get a valid
// wl_display, the Wayland server will send different events to register
// the Wayland compositor, shell, screens, input devices, ...
class WaylandDisplay {
 public:
  static inline WaylandDisplay* GetInstance() { return instance_; }

  // Returns a list of the registered screens.
  inline std::list<WaylandScreen*> GetScreenList() const { return screen_list_; }
  inline WaylandScreen* PrimaryScreen() const { return primary_screen_ ; }

  // Accessors
  inline wl_display* Display() const { return display_; }
  inline wl_compositor* Compositor() { return compositor_; }
  inline wl_registry* Registry() const { return registry_; }
  inline wl_shell* Shell() const { return shell_; }
  inline wl_shm* Shm() const { return shm_; }

 private:
  WaylandDisplay();
  virtual ~WaylandDisplay();
  void Terminate();

  // This handler resolves all server events used in initialization. It also
  // handles input device registration, screen registration.
  static void DisplayHandleGlobal(
      void *data,
      struct wl_registry *registry,
      uint32_t name,
      const char *interface,
      uint32_t version);

  static void DisplayHandleGlobalRemove(
      void *data,
      struct wl_registry *registry,
      uint32_t name);

  // WaylandDisplay manages the memory of all these pointers.
  wl_display* display_;
  wl_registry* registry_;
  wl_compositor* compositor_;
  wl_shell* shell_;
  wl_shm* shm_;
  WaylandScreen* primary_screen_;

  std::list<WaylandScreen*> screen_list_;
  std::list<WaylandInputDevice*> input_list_;
  static WaylandDisplay* instance_;

  friend class OzoneDisplay;
  DISALLOW_COPY_AND_ASSIGN(WaylandDisplay);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_DISPLAY_H_
