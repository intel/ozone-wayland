// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_CURSOR_H_
#define OZONE_WAYLAND_CURSOR_H_

#include "ozone/wayland_input_device.h"
#include <wayland-cursor.h>

namespace ui {
class WaylandCursorData;

class WaylandCursor {
public:
  enum CursorType {
    CURSOR_UNSET,
    CURSOR_BOTTOM_LEFT,
    CURSOR_BOTTOM_RIGHT,
    CURSOR_BOTTOM,
    CURSOR_DRAGGING,
    CURSOR_LEFT_PTR,
    CURSOR_LEFT,
    CURSOR_RIGHT,
    CURSOR_TOP_LEFT,
    CURSOR_TOP_RIGHT,
    CURSOR_TOP,
    CURSOR_IBEAM,
    CURSOR_HAND1
   };

  WaylandCursor(wl_compositor* compositor, wl_shm* shm);
  virtual ~WaylandCursor();

  void Update(CursorType type, uint32_t serial);
  struct wl_surface* GetSurface() const { return pointer_surface_; }

  wl_pointer* GetInputPointer() { return input_pointer_; }
  void SetInputPointer(wl_pointer* pointer);

private:
  wl_pointer *input_pointer_;
  struct wl_surface *pointer_surface_;
  DISALLOW_COPY_AND_ASSIGN(WaylandCursor);
};

}  // namespace ui

#endif  // OZONE_WAYLAND_DISPLAY_H_
