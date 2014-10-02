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
  virtual ~IVIShellSurface();

  virtual void InitializeShellSurface(WaylandWindow* window,
                                      WaylandWindow::ShellType type) OVERRIDE;
  virtual void UpdateShellSurface(WaylandWindow::ShellType type,
                                  WaylandShellSurface* shell_parent,
                                  unsigned x,
                                  unsigned y) OVERRIDE;
  virtual void SetWindowTitle(const base::string16& title) OVERRIDE;
  virtual void Maximize() OVERRIDE;
  virtual void Minimize() OVERRIDE;
  virtual void Unminimize() OVERRIDE;
  virtual bool IsMinimized() const OVERRIDE;

 private:
  ivi_surface* ivi_surface_;
  int ivi_surface_id_;
  static int last_ivi_surface_id_;
  DISALLOW_COPY_AND_ASSIGN(IVIShellSurface);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_SHELL_IVI_SHELL_SURFACE_H_
