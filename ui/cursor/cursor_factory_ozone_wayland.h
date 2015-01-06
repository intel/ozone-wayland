// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_CURSOR_CURSOR_FACTORY_OZONE_WAYLAND_H_
#define OZONE_UI_CURSOR_CURSOR_FACTORY_OZONE_WAYLAND_H_

#include "ui/ozone/public/cursor_factory_ozone.h"

namespace ui {

class CursorFactoryOzoneWayland : public CursorFactoryOzone {
 public:
  CursorFactoryOzoneWayland();
  ~CursorFactoryOzoneWayland() override;

  PlatformCursor GetDefaultCursor(int type) override;
  PlatformCursor CreateImageCursor(const SkBitmap& bitmap,
                                   const gfx::Point& hotspot) override;
  void RefImageCursor(PlatformCursor cursor) override;
  void UnrefImageCursor(PlatformCursor cursor) override;
};

}  // namespace ui

#endif  // OZONE_UI_CURSOR_CURSOR_FACTORY_OZONE_WAYLAND_H_
