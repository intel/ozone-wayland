// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/shell/shell.h"

#include "base/logging.h"
#include "ozone/wayland/display.h"
#include "ozone/wayland/shell/wl_shell_surface.h"
#if defined(ENABLE_XDG_SHELL)
#include "ozone/wayland/shell/xdg-shell-client-protocol.h"
#include "ozone/wayland/shell/xdg_shell_surface.h"
#endif

namespace ozonewayland {

WaylandShell::WaylandShell()
    : shell_(NULL),
      xdg_shell_(NULL) {
}

WaylandShell::~WaylandShell() {
  if (shell_)
    wl_shell_destroy(shell_);
#if defined(ENABLE_XDG_SHELL)
  if (xdg_shell_)
    xdg_shell_destroy(xdg_shell_);
#endif
}

WaylandShellSurface* WaylandShell::CreateShellSurface(WaylandWindow* window) {
  DCHECK(shell_ || xdg_shell_);
  WaylandDisplay* display = WaylandDisplay::GetInstance();
  DCHECK(display);
  WaylandShellSurface* surface = NULL;
#if defined(ENABLE_XDG_SHELL)
  if (xdg_shell_)
    surface = new XDGShellSurface();
#endif
  if (!surface)
    surface = new WLShellSurface();

  DCHECK(surface);
  surface->InitializeShellSurface(window);
  wl_surface_set_user_data(surface->GetWLSurface(), window);
  display->FlushDisplay();

  return surface;
}

void WaylandShell::Initialize(struct wl_registry *registry,
                              uint32_t name,
                              const char *interface,
                              uint32_t version) {
  if (strcmp(interface, "wl_shell") == 0) {
    DCHECK(!shell_);
    shell_ = static_cast<wl_shell*>(
        wl_registry_bind(registry, name, &wl_shell_interface, 1));
#if defined(ENABLE_XDG_SHELL)
  } else if ((strcmp(interface, "xdg_shell") == 0) && getenv("OZONE_WAYLAND_USE_XDG_SHELL")) {
      DCHECK(!xdg_shell_);
      xdg_shell_ = static_cast<xdg_shell*>(
          wl_registry_bind(registry, name, &xdg_shell_interface, 1));
      xdg_shell_use_unstable_version(xdg_shell_, XDG_SHELL_VERSION_CURRENT);

      static const xdg_shell_listener xdg_shell_listener = {
        WaylandShell::XDGHandlePing
      };
      xdg_shell_add_listener(xdg_shell_, &xdg_shell_listener, NULL);
#endif
  }
}

#if defined(ENABLE_XDG_SHELL)
void WaylandShell::XDGHandlePing(void* data,
                                 struct xdg_shell* xdg_shell,
                                 uint32_t serial) {
  xdg_shell_pong(xdg_shell, serial);
}
#endif

}  // namespace ozonewayland
