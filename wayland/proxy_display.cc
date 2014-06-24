// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/proxy_display.h"

#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ozone/ui/events/output_change_observer.h"
#include "ozone/wayland/screen.h"

namespace ozonewayland {

WaylandProxyDisplay::WaylandProxyDisplay(): screen_(NULL) {
  ui::OzoneDisplay::SetInstance(this);
}

WaylandProxyDisplay::~WaylandProxyDisplay() {
}

// TODO(vignatti): GPU process conceptually is the one that deals with hardware
// details and therefore we assume that the window system connection should
// happen in there only. There's a glitch with Chrome though, that creates its
// frame contents requiring access to the window system, before the GPU process
// even exists. In other words, Chrome runs
// BrowserMainLoop::PreMainMessageLoopRun before GpuProcessHost::Get. If the
// assumption of window system connection belongs to the GPU process is valid,
// then I believe this Chrome behavior needs to be addressed upstream.
//
// For now, we create another window system connection to look ahead the needed
// output properties that Chrome (among others) need and then close right after
// that. I haven't measured how long it takes to open a Wayland connection,
// listen all the interface the compositor sends and close it, but _for_ _sure_
// it slows down the overall initialization time of Chromium targets.
// Therefore, this is something that has to be solved in the future, moving all
// Chrome tasks after GPU process is created.
//
void WaylandProxyDisplay::LookAheadOutputGeometry() {
  wl_display* display = wl_display_connect(NULL);
  if (!display)
    return;

  static const struct wl_registry_listener registry_output = {
    WaylandProxyDisplay::DisplayHandleOutputOnly
  };

  wl_registry* registry = wl_display_get_registry(display);
  wl_registry_add_listener(registry, &registry_output, this);

  if (wl_display_roundtrip(display) > 0) {
    while (screen_->Geometry().IsEmpty())
      wl_display_roundtrip(display);

    ui::EventFactoryOzoneWayland* event_factory =
        ui::EventFactoryOzoneWayland::GetInstance();
    DCHECK(event_factory->GetOutputChangeObserver());

    unsigned width = screen_->Geometry().width();
    unsigned height = screen_->Geometry().height();
    event_factory->GetOutputChangeObserver()->OnOutputSizeChanged(width,
                                                                  height);
  }

  if (screen_) {
    delete screen_;
    screen_ = NULL;
  }

  wl_registry_destroy(registry);
  wl_display_flush(display);
  wl_display_disconnect(display);
}

// static
void WaylandProxyDisplay::DisplayHandleOutputOnly(void *data,
                                             struct wl_registry *registry,
                                             uint32_t name,
                                             const char *interface,
                                             uint32_t version) {
  WaylandProxyDisplay* disp = static_cast<WaylandProxyDisplay*>(data);

  if (strcmp(interface, "wl_output") == 0) {
    WaylandScreen* screen = new WaylandScreen(registry, name);
    disp->screen_ = screen;
  }
}

}  // namespace ozonewayland
