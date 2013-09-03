// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_WINDOW_H_
#define OZONE_WAYLAND_WINDOW_H_

#include <stdint.h>

#include "base/basictypes.h"
#include "ui/gfx/point.h"
#include "ui/gfx/rect.h"
#include "ozone/wayland/display.h"
#include <vector>

struct wl_surface;
struct wl_shell_surface;
struct wl_egl_window;

namespace ui {

class WaylandDisplay;
class WaylandSurface;
class EGLWindow;

// WaylandWindow wraps a wl_surface and some basic operations for the surface.
class WaylandWindow {
 public:

  // Creates a toplevel window.
  WaylandWindow();
  ~WaylandWindow();

  void SetParentWindow(WaylandWindow* parent_window);

  // Returns the pointer to the surface associated with the window.
  // The WaylandWindow object owns the pointer.
  WaylandSurface* surface() const { return surface_; }
  wl_shell_surface* shell_surface() const { return shell_surface_; }
  wl_egl_window* egl_window() const;

  void SetBounds(const gfx::Rect& new_bounds);
  gfx::Rect GetBounds() const;

  void RealizeAcceleratedWidget();

  static void HandleConfigure(void *data, struct wl_shell_surface *shell_surface,
      uint32_t edges, int32_t width, int32_t height);
  static void HandlePopupDone(void *data, struct wl_shell_surface *shell_surface);
  static void HandlePing(void *data, struct wl_shell_surface *shell_surface, uint32_t serial);

 private:
  static void FreeSurface(void *data, wl_callback *callback, uint32_t time);
  void HandleResize(int32_t width, int32_t height);
  void Resize();
  void SetType();

  enum WindowType{
    TYPE_TOPLEVEL,
    TYPE_FULLSCREEN,
    TYPE_TRANSIENT,
    TYPE_MENU,
    TYPE_CUSTOM
  };

  // When creating a transient window, |parent_window_| is set to point to the
  // parent of this window. We will then use |parent_window_| to align this
  // window at the specified offset in |relative_position_|.
  // |parent_window_| is not owned by this window.
  WaylandWindow* parent_window_;
  // The native wayland surface associated with this window.
  WaylandSurface* surface_;
  wl_shell_surface* shell_surface_;
  EGLWindow* window_;

  // Position relative to parent window. This is only used by
  // a transient window.
  gfx::Point relative_position_;
  gfx::Rect allocation_;
  WindowType type_;
  bool fullscreen_ :1;
  DISALLOW_COPY_AND_ASSIGN(WaylandWindow);
};

}  // namespace ui

#endif  // OZONE_WAYLAND_WINDOW_H_
