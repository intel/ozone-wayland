// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_SHELL_SHELL_H_
#define OZONE_WAYLAND_SHELL_SHELL_H_

#include <wayland-client.h>

#include "base/basictypes.h"

struct xdg_shell;
namespace ozonewayland {

class WaylandShellSurface;
class WaylandWindow;

class WaylandShell {
 public:
  WaylandShell();
  ~WaylandShell();
  // Creates shell surface for a given WaylandWindow. This can be either
  // wl_shell, xdg_shell or any shell which supports wayland protocol.
  // Ownership is passed to the caller.
  WaylandShellSurface* CreateShellSurface(WaylandWindow* parent);
  void Initialize(struct wl_registry *registry,
                  uint32_t name,
                  const char *interface,
                  uint32_t version);

 private:
  wl_shell* shell_;
  xdg_shell* xdg_shell_;
  DISALLOW_COPY_AND_ASSIGN(WaylandShell);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_SHELL_H_
