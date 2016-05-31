// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_INPUT_CURSOR_H_
#define OZONE_WAYLAND_INPUT_CURSOR_H_

#include <wayland-client.h>
#include <vector>

#include "base/macros.h"
#include "third_party/skia/include/core/SkBitmap.h"

namespace base {
class SharedMemory;
}

namespace gfx {
class Point;
}

namespace ozonewayland {

class WaylandCursor {
 public:
  WaylandCursor();
  ~WaylandCursor();

  void UpdateBitmap(const std::vector<SkBitmap>& bitmaps,
                    const gfx::Point& location,
                    uint32_t serial);
  void MoveCursor(const gfx::Point& location, uint32_t serial);

  wl_pointer* GetInputPointer() const { return input_pointer_; }
  void SetInputPointer(wl_pointer* pointer);

 private:
  bool CreateSHMBuffer(int width, int height);
  void HideCursor(uint32_t serial);

  struct wl_pointer* input_pointer_;
  struct wl_surface* pointer_surface_;
  struct wl_buffer* buffer_;
  struct wl_shm* shm_;
  base::SharedMemory* sh_memory_;
  int width_;
  int height_;
  DISALLOW_COPY_AND_ASSIGN(WaylandCursor);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_DISPLAY_H_
