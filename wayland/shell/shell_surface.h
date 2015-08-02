// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_SHELL_SHELL_SURFACE_H_
#define OZONE_WAYLAND_SHELL_SHELL_SURFACE_H_

#include <wayland-client.h>

#include "base/basictypes.h"
#include "ozone/wayland/window.h"

namespace ozonewayland {

class WaylandWindow;

class WaylandShellSurface {
 public:
  WaylandShellSurface();
  virtual ~WaylandShellSurface();

  struct wl_surface* GetWLSurface() const;

  // The implementation should initialize the shell and set up all
  // necessary callbacks.
  virtual void InitializeShellSurface(WaylandWindow* window,
                                      WaylandWindow::ShellType type) = 0;
  virtual void UpdateShellSurface(WaylandWindow::ShellType type,
                                  WaylandShellSurface* shell_parent,
                                  int x,
                                  int y) = 0;
  virtual void SetWindowTitle(const base::string16& title) = 0;
  virtual void Maximize() = 0;
  virtual void Minimize() = 0;
  virtual void Unminimize() = 0;
  virtual bool IsMinimized() const = 0;

  // static functions.
  static void PopupDone();
  static void WindowResized(void *data, unsigned width, unsigned height);
  static void WindowActivated(void *data);
  static void WindowDeActivated(void *data);

 protected:
  void FlushDisplay() const;

 private:
  struct wl_surface* surface_;
  DISALLOW_COPY_AND_ASSIGN(WaylandShellSurface);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_SHELL_SHELL_SURFACE_H_
