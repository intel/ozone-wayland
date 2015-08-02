// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/window.h"

#include "base/logging.h"
#include "ozone/wayland/display.h"
#include "ozone/wayland/egl/egl_window.h"
#include "ozone/wayland/seat.h"
#include "ozone/wayland/shell/shell.h"
#include "ozone/wayland/shell/shell_surface.h"

namespace ozonewayland {

WaylandWindow::WaylandWindow(unsigned handle) : shell_surface_(NULL),
    window_(NULL),
    type_(None),
    handle_(handle),
    allocation_(gfx::Rect(0, 0, 1, 1)) {
}

WaylandWindow::~WaylandWindow() {
  WaylandSeat* seat = WaylandDisplay::GetInstance()->PrimarySeat();
  if (seat) {
    if (seat->GetFocusWindowHandle() == handle_)
      seat->SetFocusWindowHandle(0);

    if (seat->GetGrabWindowHandle() == handle_)
      seat->SetGrabWindowHandle(0, 0);
  }

  delete window_;
  delete shell_surface_;
}

void WaylandWindow::SetShellAttributes(ShellType type) {
  if (type_ == type)
    return;

  if (!shell_surface_) {
    shell_surface_ =
        WaylandDisplay::GetInstance()->GetShell()->CreateShellSurface(this,
                                                                      type);
  }

  type_ = type;
  shell_surface_->UpdateShellSurface(type_, NULL, 0, 0);
  allocation_.set_origin(gfx::Point(0, 0));
}

void WaylandWindow::SetShellAttributes(ShellType type,
                                       WaylandShellSurface* shell_parent,
                                       int x,
                                       int y) {
  DCHECK(shell_parent && (type == POPUP));

  if (!shell_surface_) {
    shell_surface_ =
        WaylandDisplay::GetInstance()->GetShell()->CreateShellSurface(this,
                                                                      type);
  }

  type_ = type;
  shell_surface_->UpdateShellSurface(type_, shell_parent, x, y);
  allocation_.set_origin(gfx::Point(x, y));
}

void WaylandWindow::SetWindowTitle(const base::string16& title) {
  shell_surface_->SetWindowTitle(title);
}

void WaylandWindow::Maximize() {
  if (type_ != FULLSCREEN)
    shell_surface_->Maximize();
}

void WaylandWindow::Minimize() {
  shell_surface_->Minimize();
}

void WaylandWindow::Show() {
  WaylandSeat* seat = WaylandDisplay::GetInstance()->PrimarySeat();
  seat->SetFocusWindowHandle(handle_);
}

void WaylandWindow::Hide() {
  WaylandSeat* seat = WaylandDisplay::GetInstance()->PrimarySeat();
  if (seat && seat->GetFocusWindowHandle() == handle_)
    seat->SetFocusWindowHandle(0);
}

void WaylandWindow::Restore() {
  // If window is created as fullscreen, we don't set/restore any window states
  // like Maximize etc.
  if (type_ != FULLSCREEN)
    shell_surface_->UpdateShellSurface(type_, NULL, 0, 0);
}

void WaylandWindow::SetFullscreen() {
  if (type_ != FULLSCREEN)
    shell_surface_->UpdateShellSurface(FULLSCREEN, NULL, 0, 0);
}

void WaylandWindow::RealizeAcceleratedWidget() {
  if (!shell_surface_) {
    LOG(ERROR) << "Shell type not set. Setting it to TopLevel";
    SetShellAttributes(TOPLEVEL);
  }

  if (!window_)
    window_ = new EGLWindow(shell_surface_->GetWLSurface(),
                            allocation_.width(),
                            allocation_.height());
}

wl_egl_window* WaylandWindow::egl_window() const {
  DCHECK(window_);
  return window_->egl_window();
}

void WaylandWindow::Resize(unsigned width, unsigned height) {
  if ((allocation_.width() == width) && (allocation_.height() == height))
    return;

  allocation_ = gfx::Rect(allocation_.x(), allocation_.y(), width, height);
  if (!shell_surface_ || !window_)
    return;

  window_->Resize(allocation_.width(), allocation_.height());
  WaylandDisplay::GetInstance()->FlushDisplay();
}

void WaylandWindow::Move(ShellType type, WaylandShellSurface* shell_parent,
                         const gfx::Rect& rect) {
  int x = rect.x();
  int y = rect.y();
  if ((allocation_.x() == x) && (allocation_.y() == y))
    return;

  if (!shell_surface_ || !window_) {
    SetShellAttributes(type, shell_parent, x, y);
    return;
  }

  int move_x = x - allocation_.x();
  int move_y = y - allocation_.y();
  allocation_ = rect;
  window_->Move(allocation_.width(), allocation_.height(), move_x, move_y);
}

void WaylandWindow::AddRegion(int left, int top, int right, int bottom) {
  wl_compositor* com = WaylandDisplay::GetInstance()->GetCompositor();
  struct wl_region *region = wl_compositor_create_region(com);
  wl_region_add(region, left, top, right, bottom);
  wl_surface_set_input_region(shell_surface_->GetWLSurface(), region);
  wl_surface_set_opaque_region(shell_surface_->GetWLSurface(), region);
  wl_region_destroy(region);
}

void WaylandWindow::SubRegion(int left, int top, int right, int bottom) {
  wl_compositor* com = WaylandDisplay::GetInstance()->GetCompositor();
  struct wl_region *region = wl_compositor_create_region(com);
  wl_region_subtract(region, left, top, right, bottom);
  wl_surface_set_opaque_region(shell_surface_->GetWLSurface(), region);
  wl_region_destroy(region);
}

}  // namespace ozonewayland
