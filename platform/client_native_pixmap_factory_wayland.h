// Copyright 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_CLIENT_NATIVE_PIXMAP_FACTORY_WAYLAND_H_
#define OZONE_CLIENT_NATIVE_PIXMAP_FACTORY_WAYLAND_H_

namespace ui {

class ClientNativePixmapFactory;

// Constructor hook for use in constructor_list.cc
ClientNativePixmapFactory* CreateClientNativePixmapFactoryWayland();

}  // namespace ui

#endif  // UI_OZONE_CLIENT_NATIVE_PIXMAP_FACTORY_WAYLAND_H_
