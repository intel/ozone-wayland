// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_SHELL_XDG_SURFACE_H_
#define OZONE_WAYLAND_SHELL_XDG_SURFACE_H_

#include "ozone/wayland/shell/xdg-shell-client-protocol.h"
#include "ozone/wayland/shell_surface.h"

namespace ozonewayland {

class WaylandSurface;
class WaylandWindow;

class XDGShellSurface : public WaylandShellSurface {
 public:
  XDGShellSurface();
  virtual ~XDGShellSurface();

  virtual void InitializeShellSurface(WaylandWindow* window) OVERRIDE;
  virtual void UpdateShellSurface(WaylandWindow::ShellType type,
                                  WaylandShellSurface* shell_parent,
                                  unsigned x,
                                  unsigned y) OVERRIDE;
  virtual void SetWindowTitle(const base::string16& title) OVERRIDE;
  virtual void Maximize() OVERRIDE;
  virtual void Minimize() OVERRIDE;

  static void HandleConfigure(void* data,
                              struct xdg_surface* xdg_surface,
                              uint32_t edges,
                              int32_t width,
                              int32_t height);
  static void HandlePing(void* data,
                         struct xdg_surface* shell_surface,
                         uint32_t serial);

  static void HandlePopupPopupDone(void* data,
                                   struct xdg_popup* xdg_popup,
                                   uint32_t serial);
  static void HandlePopupPing(void* data,
                              struct xdg_popup* xdg_popup,
                              uint32_t serial);

 private:
  xdg_surface* xdg_surface_;
  xdg_popup* xdg_popup_;
  bool maximized_;
  DISALLOW_COPY_AND_ASSIGN(XDGShellSurface);
};


}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_SHELL_XDG_SURFACE_H_
