// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/shell_surface.h"

#include "ozone/wayland/display.h"
#include "ozone/wayland/surface.h"
#include "base/logging.h"

namespace ozonewayland {

WaylandShellSurface::WaylandShellSurface(WaylandWindow* window)
    : surface_(NULL),
      shell_surface_(NULL)
{
  WaylandDisplay* display = WaylandDisplay::GetInstance();
  if (!display)
      return;

  surface_ = new WaylandSurface();
  if (display->shell()) {
    shell_surface_ = wl_shell_get_shell_surface(
        display->shell(),
        surface_->wlSurface());
    UpdateShellSurface(window->Type());
  }

  if (shell_surface_)
  {
    static const wl_shell_surface_listener shell_surface_listener = {
      WaylandShellSurface::HandlePing,
      WaylandShellSurface::HandleConfigure,
      WaylandShellSurface::HandlePopupDone
    };

  wl_shell_surface_add_listener(shell_surface_, &shell_surface_listener, window);
  }
}

WaylandShellSurface::~WaylandShellSurface() {

  if (shell_surface_)
  {
    wl_shell_surface_destroy(shell_surface_);
    shell_surface_ = NULL;
  }

  if (surface_)
  {
    delete surface_;
    surface_ = NULL;
  }
}

void WaylandShellSurface::UpdateShellSurface(WaylandWindow::ShellType type) const
{
  switch (type) {
  case WaylandWindow::TOPLEVEL:
    wl_shell_surface_set_toplevel(shell_surface_);
    break;
  case WaylandWindow::FULLSCREEN:
  case WaylandWindow::TRANSIENT:
  case WaylandWindow::MENU:
  case WaylandWindow::CUSTOM:
      NOTREACHED() << "UnSupported Shell Type.";
    break;
    default:
      break;
  }
}

void WaylandShellSurface::HandleConfigure(void *data,
                                          struct wl_shell_surface *shell_surface,
                                          uint32_t edges,
                                          int32_t width,
                                          int32_t height)
{
  WaylandWindow *window = static_cast<WaylandWindow*>(data);
  window->SetBounds(gfx::Rect(0, 0, width, height));
}

void WaylandShellSurface::HandlePopupDone(void *data,
                                          struct wl_shell_surface *shell_surface)
{
}

void WaylandShellSurface::HandlePing(void *data,
                                     struct wl_shell_surface *shell_surface,
                                     uint32_t serial)
{
  wl_shell_surface_pong(shell_surface, serial);
}

}  // namespace ozonewayland
