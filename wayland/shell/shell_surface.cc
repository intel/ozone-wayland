// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/shell/shell_surface.h"

#include "ozone/wayland/display.h"
#include "ozone/wayland/seat.h"

namespace ozonewayland {

WaylandShellSurface::WaylandShellSurface()
    : surface_(NULL) {
  WaylandDisplay* display = WaylandDisplay::GetInstance();
  surface_ = wl_compositor_create_surface(display->GetCompositor());
}

WaylandShellSurface::~WaylandShellSurface() {
  DCHECK(surface_);
  wl_surface_destroy(surface_);
  FlushDisplay();
}

struct wl_surface* WaylandShellSurface::GetWLSurface() const {
    return surface_;
}

void WaylandShellSurface::FlushDisplay() const {
  WaylandDisplay* display = WaylandDisplay::GetInstance();
  DCHECK(display);
  display->FlushDisplay();
}

void WaylandShellSurface::PopupDone() {
  WaylandDisplay* display = WaylandDisplay::GetInstance();
  WaylandSeat* seat = display->PrimarySeat();

  if (!seat->GetGrabWindowHandle())
    return;
  display->CloseWidget(seat->GetGrabWindowHandle());
  seat->SetGrabWindowHandle(0, 0);
}

void WaylandShellSurface::WindowResized(void* data,
                                 unsigned width,
                                 unsigned height) {
  WaylandWindow *window = static_cast<WaylandWindow*>(data);
  WaylandDisplay::GetInstance()->WindowResized(window->Handle(), width, height);
}

void WaylandShellSurface::WindowActivated(void *data) {
  WaylandWindow *window = static_cast<WaylandWindow*>(data);
  WaylandShellSurface* shellSurface = window->ShellSurface();

  WaylandDisplay* dispatcher = WaylandDisplay::GetInstance();

  if (shellSurface->IsMinimized()) {
    shellSurface->Unminimize();
    dispatcher->WindowUnminimized(window->Handle());
  } else {
    dispatcher->WindowActivated(window->Handle());
  }
}

void WaylandShellSurface::WindowDeActivated(void *data) {
  WaylandWindow *window = static_cast<WaylandWindow*>(data);
  WaylandDisplay::GetInstance()->WindowDeActivated(window->Handle());
}

}  // namespace ozonewayland
