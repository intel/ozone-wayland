// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_DESKTOP_FACTORY_WAYLAND_H_
#define OZONE_DESKTOP_FACTORY_WAYLAND_H_

#include "base/compiler_specific.h"
#include "ui/views/widget/desktop_aura/desktop_factory_ozone.h"

namespace views {

class DesktopFactoryWayland : public DesktopFactoryOzone {
 public:
  DesktopFactoryWayland();
  virtual ~DesktopFactoryWayland();

  // views::DesktopFactoryOzone
  virtual DesktopRootWindowHost* CreateRootWindowHost(
      internal::NativeWidgetDelegate* native_widget_delegate,
      DesktopNativeWidgetAura* desktop_native_widget_aura,
      const gfx::Rect& bounds) OVERRIDE;
};

} // namespace views

#endif // OZONE_DESKTOP_FACTORY_WAYLAND_H_
