// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_CURSOR_CURSOR_FACTORY_OZONE_WAYLAND_H_
#define OZONE_UI_CURSOR_CURSOR_FACTORY_OZONE_WAYLAND_H_

#include "ui/base/cursor/ozone/cursor_factory_ozone.h"

namespace ui {

class CursorFactoryOzoneWayland : public CursorFactoryOzone {
 public:
  CursorFactoryOzoneWayland();
  virtual ~CursorFactoryOzoneWayland();

  virtual PlatformCursor GetDefaultCursor(int type) OVERRIDE;
  virtual PlatformCursor CreateImageCursor(const SkBitmap& bitmap,
                                           const gfx::Point& hotspot) OVERRIDE;
  virtual void RefImageCursor(PlatformCursor cursor) OVERRIDE;
  virtual void UnrefImageCursor(PlatformCursor cursor) OVERRIDE;
  virtual void SetCursor(gfx::AcceleratedWidget widget,
                         PlatformCursor cursor) OVERRIDE;
};

}  // namespace ui

#endif  // OZONE_UI_CURSOR_CURSOR_FACTORY_OZONE_WAYLAND_H_
