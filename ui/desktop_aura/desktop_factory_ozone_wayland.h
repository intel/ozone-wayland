// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_OZONE_PLATFORM_WAYLAND_DESKTOP_FACTORY_OZONE_WAYLAND_H_
#define UI_OZONE_PLATFORM_WAYLAND_DESKTOP_FACTORY_OZONE_WAYLAND_H_

#include "ui/views/views_export.h"
#include "ui/views/widget/desktop_aura/desktop_factory_ozone.h"

namespace gfx {
class Screen;
}

namespace views {
class DesktopNativeWidgetAura;
class DesktopWindowTreeHost;

namespace internal {
class NativeWidgetDelegate;
}

class VIEWS_EXPORT DesktopFactoryOzoneWayland : public DesktopFactoryOzone {
 public:
  // Delegates implementation of DesktopWindowTreeHost::Create externally to
  // Ozone implementation.
  DesktopWindowTreeHost* CreateWindowTreeHost(
      internal::NativeWidgetDelegate* native_widget_delegate,
      DesktopNativeWidgetAura* desktop_native_widget_aura) override;

  // Delegates implementation of DesktopScreen externally to
  // Ozone implementation.
  gfx::Screen* CreateDesktopScreen() override;
};

DesktopFactoryOzone* CreateDesktopFactoryOzoneWayland();

}  // namespace views

#endif // UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_FACTORY_OZONE_H_
