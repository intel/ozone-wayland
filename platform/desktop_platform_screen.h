// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_PLATFORM_PLATFORM_SCREEN_H_
#define OZONE_PLATFORM_PLATFORM_SCREEN_H_

#include "ozone/platform/ozone_export_wayland.h"
#include "ui/gfx/geometry/point.h"

namespace ui {

class DesktopPlatformScreen {
 public:
  virtual ~DesktopPlatformScreen() { }

  virtual gfx::Point GetCursorScreenPoint() = 0;
};

}  // namespace ui

#endif  // OZONE_PLATFORM_PLATFORM_SCREEN_H_
