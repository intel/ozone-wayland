// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_DESKTOP_AURA_DESKTOP_PLATFORM_SCREEN_H__
#define OZONE_IMPL_DESKTOP_AURA_DESKTOP_PLATFORM_SCREEN_H__

#include "ui/views/views_export.h"

namespace ui {
class DesktopPlatformScreen;
}

namespace views {

// Creates a Native Screen. Caller owns the result. It's just here to avoid
// custom changes in OzonePlatform.
VIEWS_EXPORT ui::DesktopPlatformScreen* CreateDesktopPlatformScreen();

}  // namespace views

#endif  // OZONE_IMPL_DESKTOP_AURA_DESKTOP_PLATFORM_SCREEN_H__
