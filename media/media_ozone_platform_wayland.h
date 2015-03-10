// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_MEDIA_MEDIA_OZONE_PLATFORM_WAYLAND_H_
#define OZONE_MEDIA_MEDIA_OZONE_PLATFORM_WAYLAND_H_

#include "ozone/platform/ozone_export_wayland.h"

namespace media {

class MediaOzonePlatform;

OZONE_WAYLAND_EXPORT MediaOzonePlatform* CreateMediaOzonePlatformWayland();

}  // namespace media

#endif  // OZONE_MEDIA_MEDIA_OZONE_PLATFORM_WAYLAND_H_
