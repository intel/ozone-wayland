// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/desktop_factory_wayland.h"
#include "ozone/impl/desktop_root_window_host_wayland.h"

namespace ozonewayland {

DesktopFactoryWayland::DesktopFactoryWayland()
{
  LOG(INFO) << "Ozone: DesktopFactoryWayland";
}

DesktopFactoryWayland::~DesktopFactoryWayland()
{
}

views::DesktopRootWindowHost* DesktopFactoryWayland::CreateRootWindowHost(
    views::internal::NativeWidgetDelegate* native_widget_delegate,
    views::DesktopNativeWidgetAura* desktop_native_widget_aura,
    const gfx::Rect& bounds) {
  return new DesktopRootWindowHostWayland(native_widget_delegate,
                                          desktop_native_widget_aura,
                                          bounds);
}

}  // namespace ozonewayland
