// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/egl/egl_window.h"

#include "ozone/wayland/surface.h"
#include <wayland-egl.h>

namespace ui {

EGLWindow::EGLWindow(struct wl_surface* surface, int32_t width, int32_t height)
    : window_(NULL)
{
  allocation_ = gfx::Rect(0, 0, width, height);
  window_ = wl_egl_window_create(surface, allocation_.width(),
                                 allocation_.height());
}

EGLWindow::~EGLWindow() {
  if (window_)
    wl_egl_window_destroy(window_);
}

void EGLWindow::Resize(int32_t width, int32_t height)
{
  allocation_ = gfx::Rect(0, 0, width, height);
  if (!window_)
    return;

  wl_egl_window_resize(window_, allocation_.x(), allocation_.y(),
                       allocation_.width(), allocation_.height());
  //TODO(kalyan): handle window resize.
}

}  // namespace ui
