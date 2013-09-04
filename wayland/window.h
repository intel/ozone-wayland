// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_WINDOW_H_
#define OZONE_WAYLAND_WINDOW_H_

#include "ui/gfx/rect.h"
#include "ozone/wayland/display.h"

namespace ui {

class WaylandShellSurface;
class EGLWindow;
struct wl_egl_window;

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
  WaylandWindow();
  ~WaylandWindow();

  void SetShellType(ShellType type);
  void SetParentWindow(WaylandWindow* parent_window);
  void RealizeAcceleratedWidget();

  // Returns pointer to egl window associated with the window.
  // The WaylandWindow object owns the pointer.
  wl_egl_window* egl_window() const;

  void SetBounds(const gfx::Rect& new_bounds);
  gfx::Rect GetBounds() const { return allocation_; }

 private:
  WaylandShellSurface* shell_surface_;
  EGLWindow* window_;

  gfx::Rect allocation_;
  ShellType type_;
  DISALLOW_COPY_AND_ASSIGN(WaylandWindow);
};

}  // namespace ui

#endif  // OZONE_WAYLAND_WINDOW_H_
