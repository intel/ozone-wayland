// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/platform/ozone_platform_wayland.h"

namespace ui {

OzonePlatformWayland::OzonePlatformWayland() {}

OzonePlatformWayland::~OzonePlatformWayland() {}

gfx::SurfaceFactoryOzone* OzonePlatformWayland::GetSurfaceFactoryOzone() {
  return &surface_factory_ozone_;
}

ui::EventFactoryOzone* OzonePlatformWayland::GetEventFactoryOzone() {
  return &event_factory_ozone_;
}

ui::InputMethodContextFactoryOzone*
OzonePlatformWayland::GetInputMethodContextFactoryOzone() {
  return &input_method_context_factory_;
}

ui::CursorFactoryOzone* OzonePlatformWayland::GetCursorFactoryOzone() {
  return &cursor_factory_ozone_;
}

OzonePlatform* CreateOzonePlatformWayland() { return new OzonePlatformWayland; }

}  // namespace ui
