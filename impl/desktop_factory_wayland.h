// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_DESKTOP_FACTORY_WAYLAND_H_
#define OZONE_DESKTOP_FACTORY_WAYLAND_H_

#include "base/compiler_specific.h"
#include "ui/views/widget/desktop_aura/desktop_factory_ozone.h"

namespace ozonewayland {

class DesktopFactoryWayland : public views::DesktopFactoryOzone {
 public:
  DesktopFactoryWayland();
  virtual ~DesktopFactoryWayland();

  // views::DesktopFactoryOzone
  virtual views::DesktopRootWindowHost* CreateRootWindowHost(
      views::internal::NativeWidgetDelegate* native_widget_delegate,
      views::DesktopNativeWidgetAura* desktop_native_widget_aura) OVERRIDE;
};

}  // namespace ozonewayland

#endif // OZONE_DESKTOP_FACTORY_WAYLAND_H_
