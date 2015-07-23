// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/ozone_wayland_screen.h"

#include "ozone/platform/desktop_platform_screen_delegate.h"
#include "ozone/platform/window_manager_wayland.h"
#include "ozone/wayland/screen.h"

namespace ozonewayland {

OzoneWaylandScreen::OzoneWaylandScreen(
    ui::DesktopPlatformScreenDelegate* observer,
    ui::WindowManagerWayland* window_manager)
  : look_ahead_screen_(NULL),
    observer_(observer) {
  LookAheadOutputGeometry();
  window_manager->OnPlatformScreenCreated(this);
}

OzoneWaylandScreen::~OzoneWaylandScreen() {
}

gfx::Point OzoneWaylandScreen::GetCursorScreenPoint() {
  return gfx::Point();
}

void OzoneWaylandScreen::LookAheadOutputGeometry() {
  wl_display* display = wl_display_connect(NULL);
  if (!display)
    return;

  static const struct wl_registry_listener registry_output = {
    OzoneWaylandScreen::DisplayHandleOutputOnly
  };

  wl_registry* registry = wl_display_get_registry(display);
  wl_registry_add_listener(registry, &registry_output, this);

  if (wl_display_roundtrip(display) > 0) {
    while (look_ahead_screen_->Geometry().IsEmpty())
      wl_display_roundtrip(display);

    unsigned width = look_ahead_screen_->Geometry().width();
    unsigned height = look_ahead_screen_->Geometry().height();
    observer_->OnOutputSizeChanged(width, height);
  }

  if (look_ahead_screen_) {
    delete look_ahead_screen_;
    look_ahead_screen_ = NULL;
  }

  wl_registry_destroy(registry);
  wl_display_flush(display);
  wl_display_disconnect(display);
}

void OzoneWaylandScreen::DisplayHandleOutputOnly(void *data,
                                                 struct wl_registry *registry,
                                                 uint32_t name,
                                                 const char *interface,
                                                 uint32_t version) {
  OzoneWaylandScreen* disp = static_cast<OzoneWaylandScreen*>(data);

  if (strcmp(interface, "wl_output") == 0) {
    WaylandScreen* screen = new WaylandScreen(registry, name);
    disp->look_ahead_screen_ = screen;
  }
}

}  // namespace ozonewayland
