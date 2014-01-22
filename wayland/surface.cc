// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/surface.h"

#include "ozone/wayland/display.h"

namespace ozonewayland {

WaylandSurface::WaylandSurface() : surface_(NULL) {
  WaylandDisplay* display = WaylandDisplay::GetInstance();
  surface_ = wl_compositor_create_surface(display->GetCompositor());
}

WaylandSurface::~WaylandSurface() {
  if (surface_) {
    wl_surface_destroy(surface_);
    surface_ = 0;
  }
}

}  // namespace ozonewayland
