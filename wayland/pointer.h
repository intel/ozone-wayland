// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_POINTER_H_
#define OZONE_WAYLAND_POINTER_H_

#include <wayland-client.h>
#include "base/memory/scoped_ptr.h"
#include "ui/gfx/point.h"

namespace ui {

class Event;
class WaylandCursor;

class WaylandPointer {
 public:
  WaylandPointer();
  ~WaylandPointer();

  void OnSeatCapabilities(wl_seat *seat, uint32_t caps);
  WaylandCursor* Cursor() { return cursor_; }

 private:
  static void DispatchEventHelper(scoped_ptr<ui::Event> key);
  static void DispatchEvent(scoped_ptr<ui::Event> event);

  static void OnMotionNotify(
      void* data,
      wl_pointer* input_pointer,
      uint32_t time,
      wl_fixed_t sx_w,
      wl_fixed_t sy_w);

  static void OnButtonNotify(
      void* data,
      wl_pointer* input_pointer,
      uint32_t serial,
      uint32_t time,
      uint32_t button,
      uint32_t state);

  static void OnAxisNotify(
      void* data,
      wl_pointer* input_pointer,
      uint32_t time,
      uint32_t axis,
      int32_t value);

  static void OnPointerEnter(
      void* data,
      wl_pointer* input_pointer,
      uint32_t serial,
      wl_surface* surface,
      wl_fixed_t sx_w,
      wl_fixed_t sy_w);

  static void OnPointerLeave(
      void* data,
      wl_pointer* input_pointer,
      uint32_t serial,
      wl_surface* surface);

  WaylandCursor* cursor_;
  // Keeps track of the last position for the motion event. We want to
  // dispatch this with events such as wheel or button which don't have a
  // position associated on Wayland.
  gfx::Point pointer_position_;

  DISALLOW_COPY_AND_ASSIGN(WaylandPointer);
};

}  // namespace ui

#endif  // OZONE_WAYLAND_POINTER_H_
