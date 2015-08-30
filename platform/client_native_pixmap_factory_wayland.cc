// Copyright 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/platform/client_native_pixmap_factory_wayland.h"

#include "ui/ozone/common/stub_client_native_pixmap_factory.h"

namespace ui {

ClientNativePixmapFactory* CreateClientNativePixmapFactoryWayland() {
  return CreateStubClientNativePixmapFactory();
}

}  // namespace ui
