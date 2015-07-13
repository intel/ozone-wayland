// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/ozone_wayland_screen.h"

#include "ozone/ui/desktop_aura/desktop_platform_screen.h"

#include "ozone/platform/ozone_export_wayland.h"
#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ozone/ui/events/output_change_observer.h"
#include "ozone/wayland/display_poll_thread.h"
#include "ozone/wayland/egl/surface_ozone_wayland.h"
#include "ozone/wayland/input/cursor.h"
#include "ozone/wayland/screen.h"

namespace ozonewayland {

OzoneWaylandScreen::OzoneWaylandScreen() : look_ahead_screen_(NULL) {
  LookAheadOutputGeometry();
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

    ui::EventFactoryOzoneWayland* event_factory =
        ui::EventFactoryOzoneWayland::GetInstance();
    DCHECK(event_factory->GetOutputChangeObserver());

    unsigned width = look_ahead_screen_->Geometry().width();
    unsigned height = look_ahead_screen_->Geometry().height();
    event_factory->GetOutputChangeObserver()->OnOutputSizeChanged(width,
                                                                  height);
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

namespace views {

OZONE_WAYLAND_EXPORT ui::DesktopPlatformScreen* CreateDesktopPlatformScreen() {
  return new ozonewayland::OzoneWaylandScreen();
}

}
