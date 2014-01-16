// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_INPUT_CURSOR_H_
#define OZONE_WAYLAND_INPUT_CURSOR_H_

#include <wayland-client.h>
#include <wayland-cursor.h>

#include "base/basictypes.h"

namespace ozonewayland {
class WaylandSurface;

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

  explicit WaylandCursor(wl_shm* shm);
  ~WaylandCursor();

  // Destroys CursorData. WaylandDisplay is responsible for calling this as
  // needed. No other class should call this.
  static void Clear();

  void Update(CursorType type, uint32_t serial);

  wl_pointer* GetInputPointer() const { return input_pointer_; }
  void SetInputPointer(wl_pointer* pointer);

 private:
  wl_pointer* input_pointer_;
  WaylandSurface* pointer_surface_;
  DISALLOW_COPY_AND_ASSIGN(WaylandCursor);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_DISPLAY_H_
