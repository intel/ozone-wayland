// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/shell_surface.h"

#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ozone/wayland/display.h"
#include "ozone/wayland/input_device.h"
#include "ozone/wayland/shell/wl_shell_surface.h"
#if defined(ENABLE_XDG_SHELL)
#include "ozone/wayland/shell/xdg_shell_surface.h"
#endif
#include "ozone/wayland/surface.h"

namespace ozonewayland {

WaylandShellSurface::WaylandShellSurface()
    : surface_(NULL) {
  surface_ = new WaylandSurface();
}

WaylandShellSurface::~WaylandShellSurface() {
  delete surface_;
  FlushDisplay();
}

void WaylandShellSurface::FlushDisplay() const {
  WaylandDisplay* display = WaylandDisplay::GetInstance();
  DCHECK(display);
  display->FlushDisplay();
}

void WaylandShellSurface::PopupDone() {
  WaylandInputDevice* input = WaylandDisplay::GetInstance()->PrimaryInput();
  ui::EventConverterOzoneWayland* dispatcher =
      ui::EventFactoryOzoneWayland::GetInstance()->EventConverter();

  if (!input->GetGrabWindowHandle())
    return;
  dispatcher->CloseWidget(input->GetGrabWindowHandle());
  input->SetGrabWindowHandle(0, 0);
}

void WaylandShellSurface::WindowResized(void* data,
                                 unsigned width,
                                 unsigned height) {
  WaylandWindow *window = static_cast<WaylandWindow*>(data);
  ui::EventConverterOzoneWayland* dispatcher =
      ui::EventFactoryOzoneWayland::GetInstance()->EventConverter();
  dispatcher->WindowResized(window->Handle(), width, height);
}

WaylandShellSurface* WaylandShellSurface::CreateShellSurface(
    WaylandWindow* window) {
  WaylandDisplay* display = WaylandDisplay::GetInstance();
  DCHECK(display);
  WaylandShellSurface* surface = NULL;
#if defined(ENABLE_XDG_SHELL)
  if (display->xdgshell())
    surface = new XDGShellSurface();
#endif
  DCHECK(display->shell());
  if (!surface)
    surface = new WLShellSurface();

  DCHECK(surface);
  surface->InitializeShellSurface(window);

  return surface;
}

}  // namespace ozonewayland
