// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/desktop_factory_wayland.h"

#include "ozone/impl/desktop_window_tree_host_wayland.h"

namespace views {

DesktopFactoryWayland::DesktopFactoryWayland() : views::DesktopFactoryOzone() {
  LOG(INFO) << "Ozone: DesktopFactoryWayland";
  DesktopFactoryOzone::SetInstance(this);
}

DesktopFactoryWayland::~DesktopFactoryWayland() {
  DesktopFactoryOzone::SetInstance(NULL);
}

DesktopWindowTreeHost* DesktopFactoryWayland::CreateWindowTreeHost(
    internal::NativeWidgetDelegate* native_widget_delegate,
    DesktopNativeWidgetAura* desktop_native_widget_aura) {
  return new DesktopWindowTreeHostWayland(native_widget_delegate,
                                          desktop_native_widget_aura);
}

}  // namespace views
