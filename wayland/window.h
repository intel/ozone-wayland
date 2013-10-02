// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_WINDOW_H_
#define OZONE_WAYLAND_WINDOW_H_

#include "ui/gfx/rect.h"

#include <wayland-client.h>

namespace ozonewayland {

class WaylandShellSurface;
class EGLWindow;
struct wl_egl_window;

typedef unsigned WaylandWindowId;

class WaylandWindow {
 public:
  enum Shell {
    None,
    TOPLEVEL,
    FULLSCREEN,
    TRANSIENT,
    MENU,
    CUSTOM
  };

  typedef unsigned ShellType;

  // Creates a toplevel window.
  WaylandWindow(ShellType type = TOPLEVEL);
  ~WaylandWindow();

  void SetShellType(ShellType type);
  void Maximize();
  void Minimize();
  void Restore();
  void SetFullscreen();

  inline ShellType Type() const { return type_; }
  void RealizeAcceleratedWidget();
  void HandleSwapBuffers();

  // Returns pointer to egl window associated with the window.
  // The WaylandWindow object owns the pointer.
  wl_egl_window* egl_window() const;

  bool SetBounds(const gfx::Rect& new_bounds);
  inline gfx::Rect GetBounds() const { return allocation_; }

 private:
  WaylandShellSurface* shell_surface_;
  EGLWindow* window_;

  gfx::Rect allocation_;
  ShellType type_;
  DISALLOW_COPY_AND_ASSIGN(WaylandWindow);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_WINDOW_H_
