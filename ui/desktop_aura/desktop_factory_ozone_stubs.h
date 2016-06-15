// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_FACTORY_OZONE_STUBS_H
#define UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_FACTORY_OZONE_STUBS_H

#include "ui/views/widget/desktop_aura/desktop_factory_ozone.h"

namespace views {

DesktopFactoryOzone* CreateDesktopFactoryOzoneHeadless();
DesktopFactoryOzone* CreateDesktopFactoryOzoneEgltest();

}  // namespace views

#endif // UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_FACTORY_OZONE_STUBS_H
