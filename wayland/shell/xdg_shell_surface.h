// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_SHELL_XDG_SURFACE_H_
#define OZONE_WAYLAND_SHELL_XDG_SURFACE_H_

#include "ozone/wayland/shell/shell_surface.h"

struct xdg_surface;
struct xdg_popup;

namespace ozonewayland {

class WaylandSurface;
class WaylandWindow;

class XDGShellSurface : public WaylandShellSurface {
 public:
  XDGShellSurface();
  ~XDGShellSurface() override;

  void InitializeShellSurface(WaylandWindow* window,
                              WaylandWindow::ShellType type) override;
  void UpdateShellSurface(WaylandWindow::ShellType type,
                          WaylandShellSurface* shell_parent,
                          int x,
                          int y) override;
  void SetWindowTitle(const base::string16& title) override;
  void Maximize() override;
  void Minimize() override;
  void Unminimize() override;
  bool IsMinimized() const override;

  static void HandleConfigure(void* data,
                              struct xdg_surface* xdg_surface,
                              int32_t width,
                              int32_t height,
                              struct wl_array* states,
                              uint32_t serial);
  static void HandleDelete(void* data,
                           struct xdg_surface* xdg_surface);

  static void HandlePopupPopupDone(void* data,
                                   struct xdg_popup* xdg_popup,
                                   uint32_t serial);

 private:
  xdg_surface* xdg_surface_;
  xdg_popup* xdg_popup_;
  bool maximized_;
  bool minimized_;
  DISALLOW_COPY_AND_ASSIGN(XDGShellSurface);
};


}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_SHELL_XDG_SURFACE_H_
