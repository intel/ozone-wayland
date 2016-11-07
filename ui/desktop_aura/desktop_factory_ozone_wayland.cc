// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/desktop_aura/desktop_factory_ozone_wayland.h"

#include "ozone/ui/desktop_aura/desktop_screen_wayland.h"
#include "ozone/ui/desktop_aura/desktop_window_tree_host_ozone.h"

namespace views {

DesktopWindowTreeHost* DesktopFactoryOzoneWayland::CreateWindowTreeHost(
    internal::NativeWidgetDelegate* native_widget_delegate,
    DesktopNativeWidgetAura* desktop_native_widget_aura) {
  return new DesktopWindowTreeHostOzone(native_widget_delegate,
                                        desktop_native_widget_aura);
}

display::Screen* DesktopFactoryOzoneWayland::CreateDesktopScreen() {
  return new DesktopScreenWayland;
}

DesktopFactoryOzone* CreateDesktopFactoryOzoneWayland() {
  return new DesktopFactoryOzoneWayland;
}

}  // namespace views
