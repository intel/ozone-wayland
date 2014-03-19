// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_SHELL_SURFACE_H_
#define OZONE_WAYLAND_SHELL_SURFACE_H_

#include <wayland-client.h>

#include "base/basictypes.h"
#include "ozone/wayland/window.h"

namespace ozonewayland {

class WaylandSurface;
class WaylandWindow;

class WaylandShellSurface {
 public:
  // Creates shell surface for a given WaylandWindow. This can be either
  // wl_shell, xdg_shell or any shell which supports wayland protocol.
  // Ownership is passed to the caller.
  static WaylandShellSurface* CreateShellSurface(WaylandWindow* window);
  WaylandShellSurface();
  virtual ~WaylandShellSurface();

  WaylandSurface* Surface() const { return surface_; }

  virtual void UpdateShellSurface(WaylandWindow::ShellType type,
                                  WaylandShellSurface* shell_parent,
                                  unsigned x,
                                  unsigned y) const = 0;
  virtual void SetWindowTitle(const base::string16& title) = 0;
  virtual void Maximize() const = 0;
  //static functions
  static void PopupDone();
  static void WindowResized(void *data, unsigned width, unsigned height);

 protected:
  void FlushDisplay() const;
  // The implementation should initialize the shell and set up all
  // necessary callbacks.
  virtual void InitializeShellSurface(WaylandWindow* window) = 0;

 private:
  WaylandSurface* surface_;
  DISALLOW_COPY_AND_ASSIGN(WaylandShellSurface);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_SHELL_SURFACE_H_
