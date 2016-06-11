// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/desktop_aura/desktop_factory_ozone_stubs.h"

namespace views {

DesktopFactoryOzone* CreateDesktopFactoryOzoneHeadless() {
  return nullptr;
}

DesktopFactoryOzone* CreateDesktopFactoryOzoneEgltest() {
  return nullptr;
}

}  // namespace views
