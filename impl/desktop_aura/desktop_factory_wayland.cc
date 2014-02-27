// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/desktop_aura/desktop_factory_wayland.h"

#include "ozone/impl/desktop_aura/desktop_window_tree_host_wayland.h"
#include "ozone/impl/ozone_display.h"

namespace ozonewayland {

DesktopFactoryWayland::DesktopFactoryWayland() : views::DesktopFactoryOzone() {
  LOG(INFO) << "Ozone: DesktopFactoryWayland";
  views::DesktopFactoryOzone::SetInstance(this);
}

DesktopFactoryWayland::~DesktopFactoryWayland() {
  views::DesktopFactoryOzone::SetInstance(NULL);
}

views::DesktopWindowTreeHost* DesktopFactoryWayland::CreateWindowTreeHost(
    views::internal::NativeWidgetDelegate* native_widget_delegate,
    views::DesktopNativeWidgetAura* desktop_native_widget_aura) {
  return new DesktopWindowTreeHostWayland(native_widget_delegate,
                                          desktop_native_widget_aura);
}

gfx::Screen* DesktopFactoryWayland::CreateDesktopScreen() {
  return OzoneDisplay::GetInstance()->CreateDesktopScreen();
}

}  // namespace ozonewayland
