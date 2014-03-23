// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/cursor/cursor_factory_ozone_wayland.h"

namespace ui {

CursorFactoryOzoneWayland::CursorFactoryOzoneWayland() {}

CursorFactoryOzoneWayland::~CursorFactoryOzoneWayland() {}

PlatformCursor CursorFactoryOzoneWayland::GetDefaultCursor(int type) {
  return NULL;
}

PlatformCursor CursorFactoryOzoneWayland::CreateImageCursor(
    const SkBitmap& bitmap,
    const gfx::Point& hotspot) {
  return NULL;
}

void CursorFactoryOzoneWayland::RefImageCursor(PlatformCursor cursor) {
}

void CursorFactoryOzoneWayland::UnrefImageCursor(PlatformCursor cursor) {
}

void CursorFactoryOzoneWayland::SetCursor(gfx::AcceleratedWidget widget,
                                   PlatformCursor cursor) {
}

}  // namespace ui
