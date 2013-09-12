// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/egl/egl_window.h"

#include "ozone/wayland/display.h"
#include "ozone/wayland/surface.h"
#include <wayland-egl.h>

namespace ozonewayland {

EGLWindow::EGLWindow(struct wl_surface* surface, int32_t width, int32_t height)
    : window_(NULL)
{
  window_ = wl_egl_window_create(surface, width, height);
}

EGLWindow::~EGLWindow() {
  if (window_)
    wl_egl_window_destroy(window_);
}

bool EGLWindow::Resize(WaylandSurface* surface, int32_t width, int32_t height)
{
  surface->ensureFrameCallBackDone();

  int current_width, current_height;
  wl_egl_window_get_attached_size(window_, &current_width, &current_height);
  if (current_width == 0 && current_height == 0) {
    //TODO(kalyan): handle window resize. surface is not attached yet, re-create
    // a new one as resizing the surface in this case makes weston hang
    wl_egl_window_destroy(window_);
    window_ = wl_egl_window_create(surface->wlSurface(), width,
                                   height);
    wl_display_roundtrip(WaylandDisplay::GetInstance()->display());
    return false;
  } else
    wl_egl_window_resize(window_,0, 0, width, height);

    return true;
}

}  // namespace ozonewayland
