// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_SHELL_SURFACE_H_
#define OZONE_WAYLAND_SHELL_SURFACE_H_

#include "ozone/wayland/display.h"
#include "ozone/wayland/window.h"

namespace ui {

class WaylandSurface;
class WaylandWindow;

class WaylandShellSurface {
 public:
  WaylandShellSurface(WaylandWindow* window);
  ~WaylandShellSurface();

  void UpdateShellSurface(WaylandWindow::ShellType type) const;
  WaylandSurface* Surface() const { return surface_; }

  static void HandleConfigure(void *data, struct wl_shell_surface *shell_surface,
      uint32_t edges, int32_t width, int32_t height);
  static void HandlePopupDone(void *data, struct wl_shell_surface *shell_surface);
  static void HandlePing(void *data, struct wl_shell_surface *shell_surface, uint32_t serial);

 private:
  WaylandSurface* surface_;
  wl_shell_surface* shell_surface_;
  DISALLOW_COPY_AND_ASSIGN(WaylandShellSurface);
};

}  // namespace ui

#endif  // OZONE_WAYLAND_SHELL_SURFACE_H_
