// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_SURFACE_H_
#define OZONE_WAYLAND_SURFACE_H_

#include <wayland-client.h>

namespace ozonewayland {

class WaylandDisplay;

class WaylandSurface {
 public:
  WaylandSurface();
  ~WaylandSurface();
  struct wl_surface* GetWLSurface() const { return surface_; }

 private:
  struct wl_surface* surface_;
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_DISPLAY_H_
