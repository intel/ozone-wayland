// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_WINDOW_H_
#define OZONE_WAYLAND_WINDOW_H_

#include <wayland-client.h>

#include "base/strings/string16.h"
#include "ui/gfx/geometry/rect.h"

namespace ozonewayland {

class WaylandShellSurface;
class EGLWindow;
struct wl_egl_window;

typedef unsigned WaylandWindowId;

class WaylandWindow {
 public:
  enum ShellType {
    None,
    TOPLEVEL,
    FULLSCREEN,
    POPUP,
    CUSTOM
  };

  // Creates a window and maps it to handle.
  explicit WaylandWindow(unsigned handle);
  ~WaylandWindow();

  void SetShellAttributes(ShellType type);
  void SetShellAttributes(ShellType type, WaylandShellSurface* shell_parent,
                          int x, int y);
  void SetWindowTitle(const base::string16& title);
  void Maximize();
  void Minimize();
  void Restore();
  void SetFullscreen();
  void Show();
  void Hide();

  ShellType Type() const { return type_; }
  unsigned Handle() const { return handle_; }
  WaylandShellSurface* ShellSurface() const { return shell_surface_; }

  void RealizeAcceleratedWidget();

  // Returns pointer to egl window associated with the window.
  // The WaylandWindow object owns the pointer.
  wl_egl_window* egl_window() const;

  // Immediately Resizes window and flushes Wayland Display.
  void Resize(unsigned width, unsigned height);
  void Move(ShellType type,
            WaylandShellSurface* shell_parent,
            const gfx::Rect& rect);
  void AddRegion(int left, int top, int right, int bottom);
  void SubRegion(int left, int top, int right, int bottom);
  gfx::Rect GetBounds() const { return allocation_; }

 private:
  WaylandShellSurface* shell_surface_;
  EGLWindow* window_;

  ShellType type_;
  unsigned handle_;
  gfx::Rect allocation_;
  DISALLOW_COPY_AND_ASSIGN(WaylandWindow);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_WINDOW_H_
