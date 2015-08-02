// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/shell/xdg_shell_surface.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"

#include "ozone/wayland/display.h"
#include "ozone/wayland/protocol/xdg-shell-client-protocol.h"
#include "ozone/wayland/seat.h"
#include "ozone/wayland/shell/shell.h"

namespace ozonewayland {

XDGShellSurface::XDGShellSurface()
    : WaylandShellSurface(),
      xdg_surface_(NULL),
      xdg_popup_(NULL),
      maximized_(false),
      minimized_(false) {
}

XDGShellSurface::~XDGShellSurface() {
  if (xdg_surface_)
    xdg_surface_destroy(xdg_surface_);
  if (xdg_popup_)
    xdg_popup_destroy(xdg_popup_);
}

void XDGShellSurface::InitializeShellSurface(WaylandWindow* window,
                                             WaylandWindow::ShellType type) {
  DCHECK(!xdg_surface_);
  DCHECK(!xdg_popup_);
  WaylandDisplay* display = WaylandDisplay::GetInstance();
  DCHECK(display);
  WaylandShell* shell = WaylandDisplay::GetInstance()->GetShell();
  DCHECK(shell && shell->GetXDGShell());

  if (type != WaylandWindow::POPUP) {
    xdg_surface_ = xdg_shell_get_xdg_surface(shell->GetXDGShell(),
                                             GetWLSurface());

    static const xdg_surface_listener xdg_surface_listener = {
      XDGShellSurface::HandleConfigure,
      XDGShellSurface::HandleDelete
    };

    xdg_surface_add_listener(xdg_surface_,
                             &xdg_surface_listener,
                             window);

    DCHECK(xdg_surface_);
  }
}

void XDGShellSurface::UpdateShellSurface(WaylandWindow::ShellType type,
                                         WaylandShellSurface* shell_parent,
                                         int x,
                                         int y) {
  switch (type) {
  case WaylandWindow::TOPLEVEL: {
    if (maximized_) {
      xdg_surface_unset_maximized(xdg_surface_);
      maximized_ = false;
    }
    break;
  }
  case WaylandWindow::POPUP: {
    WaylandDisplay* display = WaylandDisplay::GetInstance();
    WaylandSeat* seat = display->PrimarySeat();
    wl_surface* surface = GetWLSurface();
    wl_surface* parent_surface = shell_parent->GetWLSurface();
    xdg_popup_ = xdg_shell_get_xdg_popup(display->GetShell()->GetXDGShell(),
                                         surface,
                                         parent_surface,
                                         seat->GetWLSeat(),
                                         display->GetSerial(),
                                         x,
                                         y,
                                         0);
    static const xdg_popup_listener xdg_popup_listener = {
      XDGShellSurface::HandlePopupPopupDone
    };
    xdg_popup_add_listener(xdg_popup_,
                           &xdg_popup_listener,
                           NULL);
    DCHECK(xdg_popup_);
    break;
  }
  case WaylandWindow::FULLSCREEN:
    xdg_surface_set_fullscreen(xdg_surface_,
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

void XDGShellSurface::SetWindowTitle(const base::string16& title) {
  xdg_surface_set_title(xdg_surface_, UTF16ToUTF8(title).c_str());
  WaylandShellSurface::FlushDisplay();
}

void XDGShellSurface::Maximize() {
  xdg_surface_set_maximized(xdg_surface_);
  maximized_ = true;
  WaylandShellSurface::FlushDisplay();
}

void XDGShellSurface::Minimize() {
  xdg_surface_set_minimized(xdg_surface_);
  minimized_ = true;
}

void XDGShellSurface::Unminimize() {
  minimized_ = false;
}

bool XDGShellSurface::IsMinimized() const {
  return minimized_;
}

void XDGShellSurface::HandleConfigure(void* data,
                                      struct xdg_surface* xdg_surface,
                                      int32_t width,
                                      int32_t height,
                                      struct wl_array* states,
                                      uint32_t serial) {
  wl_array_for_each_type(const unsigned int, state, states) {
    if (*state == XDG_SURFACE_STATE_ACTIVATED)
      WaylandShellSurface::WindowActivated(data);
  }

  // When a window is deactivated, states->size is 0
  if (!states->size)
    WaylandShellSurface::WindowDeActivated(data);

  if ((width > 0) && (height > 0))
    WaylandShellSurface::WindowResized(data, width, height);

  xdg_surface_ack_configure(xdg_surface, serial);
}

void XDGShellSurface::HandleDelete(void* data,
                                   struct xdg_surface* xdg_surface) {
}

void XDGShellSurface::HandlePopupPopupDone(void* data,
                                           struct xdg_popup* xdg_popup,
                                           uint32_t serial) {
  WaylandShellSurface::PopupDone();
}

}  // namespace ozonewayland
