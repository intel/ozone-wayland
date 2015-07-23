// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_PLATFORM_DESKTOP_PLATFORM_SCREEN_DELEGATE_H__
#define OZONE_PLATFORM_DESKTOP_PLATFORM_SCREEN_DELEGATE_H__

#include "ozone/platform/ozone_export_wayland.h"

namespace ui {

// A simple observer interface for all clients interested in receiving various
// output change notifications like size changes, when a new output is added,
// etc.
class OZONE_WAYLAND_EXPORT DesktopPlatformScreenDelegate {
 public:
  // Called when the current output size has changed.
  virtual void OnOutputSizeChanged(unsigned width, unsigned height) = 0;

 protected:
  virtual ~DesktopPlatformScreenDelegate() {}
};

}  // namespace ui

#endif  // OZONE_PLATFORM_DESKTOP_PLATFORM_SCREEN_DELEGATE_H__
