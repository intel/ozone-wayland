// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/desktop_aura/desktop_factory_wayland.h"

#include "ozone/ui/desktop_aura/desktop_screen_wayland.h"
#include "ozone/ui/desktop_aura/desktop_window_tree_host_wayland.h"
#include "ui/ozone/public/ozone_platform.h"
#include "ui/platform_window/platform_window.h"
#include "ui/platform_window/platform_window_delegate.h"

namespace views {

DesktopFactoryWayland::DesktopFactoryWayland() : views::DesktopFactoryOzone(),
    desktop_screen_(NULL) {
  LOG(INFO) << "Ozone: DesktopFactoryWayland";
  DesktopFactoryOzone::SetInstance(this);
}

DesktopFactoryWayland::~DesktopFactoryWayland() {
  DesktopFactoryOzone::SetInstance(NULL);
  delete desktop_screen_;
}

DesktopWindowTreeHost* DesktopFactoryWayland::CreateWindowTreeHost(
    internal::NativeWidgetDelegate* native_widget_delegate,
    DesktopNativeWidgetAura* desktop_native_widget_aura) {
  return new DesktopWindowTreeHostWayland(native_widget_delegate,
                                          desktop_native_widget_aura);
}

gfx::Screen* DesktopFactoryWayland::CreateDesktopScreen() {
  if (!desktop_screen_) {
    desktop_screen_ = new DesktopScreenWayland();
    ui::OzonePlatform::GetInstance()->CreatePlatformWindow(NULL, gfx::Rect());
  }

  return desktop_screen_;
}

}  // namespace views
