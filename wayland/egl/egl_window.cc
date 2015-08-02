// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/egl/egl_window.h"

#include <EGL/egl.h>
#include <wayland-egl.h>

namespace ozonewayland {

EGLWindow::EGLWindow(struct wl_surface* surface, int32_t width, int32_t height)
    : window_(NULL) {
  window_ = wl_egl_window_create(surface, width, height);
}

EGLWindow::~EGLWindow() {
  wl_egl_window_destroy(window_);
}

void EGLWindow::Resize(int width, int height) {
  wl_egl_window_resize(window_, width, height, 0, 0);
}

void EGLWindow::Move(int width, int height, int x, int y) {
  wl_egl_window_resize(window_, width, height, x, y);
}

}  // namespace ozonewayland
