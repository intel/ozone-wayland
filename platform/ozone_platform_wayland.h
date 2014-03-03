// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_PLATFORM_OZONE_PLATFORM_WAYLAND_H_
#define OZONE_PLATFORM_OZONE_PLATFORM_WAYLAND_H_

#if defined(TOOLKIT_VIEWS) && !defined(OS_CHROMEOS)
#include "ozone/impl/desktop_aura/desktop_factory_wayland.h"
#endif
#include "ozone/impl/ozone_display.h"
#include "ozone/impl/surface_factory_wayland.h"
#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ozone/ui/ime/input_method_context_factory_wayland.h"
#include "ui/ozone/ozone_platform.h"

namespace ui {

// OzonePlatform for Wayland
//
// This platform is Linux with the wayland display server.
class OzonePlatformWayland : public OzonePlatform {
 public:
  OzonePlatformWayland();
  virtual ~OzonePlatformWayland();

  virtual gfx::SurfaceFactoryOzone* GetSurfaceFactoryOzone() OVERRIDE;
  virtual ui::EventFactoryOzone* GetEventFactoryOzone() OVERRIDE;
  virtual ui::InputMethodContextFactoryOzone*
      GetInputMethodContextFactoryOzone() OVERRIDE;

 private:
  ozonewayland::OzoneDisplay display_;
  ozonewayland::SurfaceFactoryWayland surface_factory_ozone_;
  ui::EventFactoryOzoneWayland event_factory_ozone_;
  ui::InputMethodContextFactoryWayland input_method_context_factory_;
#if defined(TOOLKIT_VIEWS) && !defined(OS_CHROMEOS)
  ozonewayland::DesktopFactoryWayland desktop_factory_ozone_;
#endif

  DISALLOW_COPY_AND_ASSIGN(OzonePlatformWayland);
};

// Constructor hook for use in ozone_platform_list.cc
OZONE_WAYLAND_EXPORT OzonePlatform* CreateOzonePlatformWayland();

}  // namespace ui

#endif  // OZONE_PLATFORM_OZONE_PLATFORM_WAYLAND_H_
