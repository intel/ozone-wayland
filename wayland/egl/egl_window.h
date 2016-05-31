// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_EGL_EGL_WINDOW_H_
#define OZONE_WAYLAND_EGL_EGL_WINDOW_H_

#include <wayland-client.h>

#include "base/macros.h"

struct wl_egl_window;

namespace ozonewayland {

class WaylandSurface;
class EGLWindow {
 public:
  EGLWindow(struct wl_surface* surface, int32_t width, int32_t height);
  ~EGLWindow();

  wl_egl_window* egl_window() const { return window_; }
  void Resize(int width, int height);
  void Move(int width, int height, int x, int y);

 private:
  struct wl_egl_window* window_;
  DISALLOW_COPY_AND_ASSIGN(EGLWindow);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_EGL_EGL_WINDOW_H_
