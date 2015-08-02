// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/shell/ivi_shell_surface.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"

#include "ozone/wayland/display.h"
#include "ozone/wayland/protocol/ivi-application-client-protocol.h"
#include "ozone/wayland/shell/shell.h"
#define IVI_SURFACE_ID 7000

namespace ozonewayland {

int IVIShellSurface::last_ivi_surface_id_ = IVI_SURFACE_ID;

IVIShellSurface::IVIShellSurface()
    : WaylandShellSurface(),
      ivi_surface_(NULL),
      ivi_surface_id_(IVI_SURFACE_ID) {
}

IVIShellSurface::~IVIShellSurface() {
  ivi_surface_destroy(ivi_surface_);
}

void IVIShellSurface::InitializeShellSurface(WaylandWindow* window,
                                             WaylandWindow::ShellType type) {
  DCHECK(!ivi_surface_);
  WaylandDisplay* display = WaylandDisplay::GetInstance();
  DCHECK(display);
  WaylandShell* shell = WaylandDisplay::GetInstance()->GetShell();
  DCHECK(shell && shell->GetIVIShell());
  char *env;
  if ((env = getenv("OZONE_WAYLAND_IVI_SURFACE_ID")))
    ivi_surface_id_ = atoi(env);
  else
    ivi_surface_id_ = last_ivi_surface_id_ + 1;
  ivi_surface_ = ivi_application_surface_create(
                     shell->GetIVIShell(), ivi_surface_id_, GetWLSurface());
  last_ivi_surface_id_ = ivi_surface_id_;

  DCHECK(ivi_surface_);
}

void IVIShellSurface::UpdateShellSurface(WaylandWindow::ShellType type,
                                         WaylandShellSurface* shell_parent,
                                         int x,
                                         int y) {
  WaylandShellSurface::FlushDisplay();
}

void IVIShellSurface::SetWindowTitle(const base::string16& title) {
}

void IVIShellSurface::Maximize() {
}

void IVIShellSurface::Minimize() {
}

void IVIShellSurface::Unminimize() {
}

bool IVIShellSurface::IsMinimized() const {
  return false;
}


}  // namespace ozonewayland
