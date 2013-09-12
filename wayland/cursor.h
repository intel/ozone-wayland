// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_CURSOR_H_
#define OZONE_WAYLAND_CURSOR_H_

#include "ozone/wayland/input_device.h"
#include <wayland-cursor.h>

namespace ozonewayland {
class WaylandCursorData;
class WaylandSurface;
class WaylandDisplay;

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

  WaylandCursor(wl_shm* shm);
  virtual ~WaylandCursor();

  void Update(CursorType type, uint32_t serial);

  wl_pointer* GetInputPointer() { return input_pointer_; }
  void SetInputPointer(wl_pointer* pointer);

private:
  void ValidateBuffer(CursorType type, uint32_t serial);
  static void Clear();
  wl_pointer* input_pointer_;
  WaylandSurface* pointer_surface_;
  struct wl_buffer* buffer_;
  int width_;
  int height_;
  CursorType type_;
  friend class WaylandDisplay;
  DISALLOW_COPY_AND_ASSIGN(WaylandCursor);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_DISPLAY_H_
