// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/shell_surface.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"

#include "ozone/ui/events/event_converter_ozone_wayland.h"
#include "ozone/wayland/display.h"
#include "ozone/wayland/input_device.h"
#include "ozone/wayland/surface.h"

namespace ozonewayland {

WaylandShellSurface::WaylandShellSurface(WaylandWindow* window)
    : surface_(NULL),
      shell_surface_(NULL) {
  WaylandDisplay* display = WaylandDisplay::GetInstance();
  DCHECK(display && display->shell());

  surface_ = new WaylandSurface();
  shell_surface_ = wl_shell_get_shell_surface(display->shell(),
                                              surface_->wlSurface());

  static const wl_shell_surface_listener shell_surface_listener = {
    WaylandShellSurface::HandlePing,
    WaylandShellSurface::HandleConfigure,
    WaylandShellSurface::HandlePopupDone
  };

  wl_shell_surface_add_listener(shell_surface_,
                                &shell_surface_listener,
                                window);

  DCHECK(shell_surface_);
}

WaylandShellSurface::~WaylandShellSurface() {
  wl_shell_surface_destroy(shell_surface_);
  delete surface_;
  FlushDisplay();
}

void WaylandShellSurface::UpdateShellSurface(WaylandWindow::ShellType type,
                                             WaylandShellSurface* shell_parent,
                                             unsigned x,
                                             unsigned y) const {
  switch (type) {
  case WaylandWindow::TOPLEVEL:
    wl_shell_surface_set_toplevel(shell_surface_);
    break;
  case WaylandWindow::POPUP: {
    WaylandDisplay* display = WaylandDisplay::GetInstance();
    WaylandInputDevice* input_device = display->PrimaryInput();
    wl_surface* parent_surface = shell_parent->Surface()->wlSurface();
    wl_shell_surface_set_popup(shell_surface_,
                               input_device->GetInputSeat(),
                               display->GetSerial(),
                               parent_surface,
                               x,
                               y,
                               0);
    break;
  }
  case WaylandWindow::FULLSCREEN:
    wl_shell_surface_set_fullscreen(shell_surface_,
                                    WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT,
                                    0,
                                    NULL);
    break;
  case WaylandWindow::CUSTOM:
      NOTREACHED() << "Unsupported shell type: " << type;
    break;
    default:
      break;
  }

  FlushDisplay();
}

void WaylandShellSurface::SetWindowTitle(const base::string16& title) {
  wl_shell_surface_set_title(shell_surface_, UTF16ToUTF8(title).c_str());
  FlushDisplay();
}

void WaylandShellSurface::HandleConfigure(void *data,
                                          struct wl_shell_surface *surface,
                                          uint32_t edges,
                                          int32_t width,
                                          int32_t height) {
}

void WaylandShellSurface::HandlePopupDone(void *data,
                                          struct wl_shell_surface *surface) {
  WaylandInputDevice* input = WaylandDisplay::GetInstance()->PrimaryInput();
  EventConverterOzoneWayland* dispatcher =
      EventConverterOzoneWayland::GetInstance();

  if (!input->GetGrabWindowHandle())
    return;
  dispatcher->CloseWidget(input->GetGrabWindowHandle());
  input->SetGrabWindowHandle(0, 0);
}

void WaylandShellSurface::HandlePing(void *data,
                                     struct wl_shell_surface *shell_surface,
                                     uint32_t serial) {
  wl_shell_surface_pong(shell_surface, serial);
}

void WaylandShellSurface::FlushDisplay() const {
  WaylandDisplay* display = WaylandDisplay::GetInstance();
  DCHECK(display);
  display->FlushDisplay();
}

}  // namespace ozonewayland
