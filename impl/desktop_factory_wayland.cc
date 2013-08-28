// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/desktop_factory_wayland.h"
#include "ozone/impl/desktop_root_window_host_wayland.h"

namespace views {

DesktopFactoryWayland::DesktopFactoryWayland()
{
  LOG(INFO) << "Ozone: DesktopFactoryWayland";
}

DesktopFactoryWayland::~DesktopFactoryWayland()
{
}

DesktopRootWindowHost* DesktopFactoryWayland::CreateRootWindowHost(
    internal::NativeWidgetDelegate* native_widget_delegate,
    DesktopNativeWidgetAura* desktop_native_widget_aura,
    const gfx::Rect& bounds) {
  return new DesktopRootWindowHostWayland(native_widget_delegate,
                                          desktop_native_widget_aura,
                                          bounds);
}

}  // namespace views
