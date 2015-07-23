// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_OZONE_WAYLAND_SCREEN_H_
#define OZONE_WAYLAND_OZONE_WAYLAND_SCREEN_H_

#include <wayland-client.h>

#include "ozone/platform/desktop_platform_screen.h"

namespace ui {
class DesktopPlatformScreenDelegate;
class WindowManagerWayland;
}

namespace ozonewayland {

class WaylandScreen;

class OzoneWaylandScreen : public ui::DesktopPlatformScreen {
 public:
  OzoneWaylandScreen(ui::DesktopPlatformScreenDelegate* observer,
                     ui::WindowManagerWayland* window_manager);
  ~OzoneWaylandScreen() override;

  // PlatformScreen:
  gfx::Point GetCursorScreenPoint() override;
  ui::DesktopPlatformScreenDelegate* GetDelegate() const { return observer_; }

 private:
  void LookAheadOutputGeometry();
  // This handler resolves only screen registration. In general you don't want
  // to use this but the one below.
  static void DisplayHandleOutputOnly(
      void *data,
      struct wl_registry *registry,
      uint32_t name,
      const char *interface,
      uint32_t version);
  WaylandScreen* look_ahead_screen_;
  ui::DesktopPlatformScreenDelegate* observer_;
  DISALLOW_COPY_AND_ASSIGN(OzoneWaylandScreen);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_OZONE_WAYLAND_SCREEN_H_
