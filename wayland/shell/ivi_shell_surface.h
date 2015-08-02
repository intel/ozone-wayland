// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_SHELL_IVI_SHELL_SURFACE_H_
#define OZONE_WAYLAND_SHELL_IVI_SHELL_SURFACE_H_

#include "ozone/wayland/shell/shell_surface.h"

struct ivi_surface;

namespace ozonewayland {

class WaylandSurface;
class WaylandWindow;

class IVIShellSurface : public WaylandShellSurface {
 public:
  IVIShellSurface();
  ~IVIShellSurface() override;

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

 private:
  ivi_surface* ivi_surface_;
  int ivi_surface_id_;
  static int last_ivi_surface_id_;
  DISALLOW_COPY_AND_ASSIGN(IVIShellSurface);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_SHELL_IVI_SHELL_SURFACE_H_
