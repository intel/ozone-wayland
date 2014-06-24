// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_PLATFORM_OZONE_PLATFORM_WAYLAND_H_
#define OZONE_PLATFORM_OZONE_PLATFORM_WAYLAND_H_

#include "ozone/platform/ozone_export_wayland.h"

namespace ui {

class OzonePlatform;

// Constructor hook for use in ozone_platform_list.cc
OZONE_WAYLAND_EXPORT OzonePlatform* CreateOzonePlatformWayland();

}  // namespace ui

#endif  // OZONE_PLATFORM_OZONE_PLATFORM_WAYLAND_H_
