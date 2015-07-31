// Copyright (c) 2014 Noser Engineering AG. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_INPUT_TOUCHSCREEN_H_
#define OZONE_WAYLAND_INPUT_TOUCHSCREEN_H_

#include "ozone/wayland/display.h"
#include "ui/gfx/geometry/point.h"

namespace ozonewayland {

class WaylandWindow;

class WaylandTouchscreen {
 public:
  WaylandTouchscreen();
  ~WaylandTouchscreen();

  void OnSeatCapabilities(wl_seat *seat, uint32_t caps);

 private:
  static void OnTouchDown(
      void *data,
      struct wl_touch *wl_touch,
      uint32_t serial,
      uint32_t time,
      struct wl_surface *surface,
      int32_t id,
      wl_fixed_t x,
      wl_fixed_t y);

  static void OnTouchUp(
      void *data,
      struct wl_touch *wl_touch,
      uint32_t serial,
      uint32_t time,
      int32_t id);

  static void OnTouchMotion(
      void *data,
      struct wl_touch *wl_touch,
      uint32_t time,
      int32_t id,
      wl_fixed_t x,
      wl_fixed_t y);

  static void OnTouchFrame(
      void *data,
      struct wl_touch *wl_touch);

  static void OnTouchCancel(
      void *data,
      struct wl_touch *wl_touch);

  WaylandDisplay* dispatcher_;
  gfx::Point pointer_position_;
  struct wl_touch* wl_touch_;

  DISALLOW_COPY_AND_ASSIGN(WaylandTouchscreen);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_INPUT_POINTER_H_
