// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/shell/wl_shell_surface.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"

#include "ozone/wayland/display.h"
#include "ozone/wayland/seat.h"
#include "ozone/wayland/shell/shell.h"

namespace ozonewayland {

WLShellSurface::WLShellSurface()
    : WaylandShellSurface(),
      shell_surface_(NULL) {
}

WLShellSurface::~WLShellSurface() {
  wl_shell_surface_destroy(shell_surface_);
}

void WLShellSurface::InitializeShellSurface(WaylandWindow* window,
                                            WaylandWindow::ShellType type) {
  DCHECK(!shell_surface_);
  WaylandDisplay* display = WaylandDisplay::GetInstance();
  DCHECK(display);
  WaylandShell* shell = WaylandDisplay::GetInstance()->GetShell();
  DCHECK(shell && shell->GetWLShell());
  shell_surface_ = wl_shell_get_shell_surface(shell->GetWLShell(),
                                              GetWLSurface());

  static const wl_shell_surface_listener shell_surface_listener = {
    WLShellSurface::HandlePing,
    WLShellSurface::HandleConfigure,
    WLShellSurface::HandlePopupDone
  };

  wl_shell_surface_add_listener(shell_surface_,
                                &shell_surface_listener,
                                window);

  DCHECK(shell_surface_);
}

void WLShellSurface::UpdateShellSurface(WaylandWindow::ShellType type,
                                        WaylandShellSurface* shell_parent,
                                        int x,
                                        int y) {
  switch (type) {
  case WaylandWindow::TOPLEVEL:
    wl_shell_surface_set_toplevel(shell_surface_);
    break;
  case WaylandWindow::POPUP: {
    WaylandDisplay* display = WaylandDisplay::GetInstance();
    WaylandSeat* seat = display->PrimarySeat();
    wl_surface* parent_surface = shell_parent->GetWLSurface();
    wl_shell_surface_set_popup(shell_surface_,
                               seat->GetWLSeat(),
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

  WaylandShellSurface::FlushDisplay();
}

void WLShellSurface::SetWindowTitle(const base::string16& title) {
  wl_shell_surface_set_title(shell_surface_, UTF16ToUTF8(title).c_str());
  WaylandShellSurface::FlushDisplay();
}

void WLShellSurface::Maximize() {
  wl_shell_surface_set_maximized(shell_surface_, NULL);
  WaylandShellSurface::FlushDisplay();
}

void WLShellSurface::Minimize() {
}

void WLShellSurface::Unminimize() {
}

bool WLShellSurface::IsMinimized() const {
  return false;
}

void WLShellSurface::HandleConfigure(void* data,
                                     struct wl_shell_surface* surface,
                                     uint32_t edges,
                                     int32_t width,
                                     int32_t height) {
  WaylandShellSurface::WindowResized(data, width, height);
}

void WLShellSurface::HandlePopupDone(void* data,
                                     struct wl_shell_surface* surface) {
  WaylandShellSurface::PopupDone();
}

void WLShellSurface::HandlePing(void* data,
                                struct wl_shell_surface* shell_surface,
                                uint32_t serial) {
  wl_shell_surface_pong(shell_surface, serial);
}

}  // namespace ozonewayland
