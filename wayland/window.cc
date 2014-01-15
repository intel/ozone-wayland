// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/window.h"

#include "base/logging.h"
#include "ozone/wayland/display.h"
#include "ozone/wayland/egl/egl_window.h"
#include "ozone/wayland/input_device.h"
#include "ozone/wayland/shell_surface.h"
#include "ozone/wayland/surface.h"

namespace ozonewayland {

WaylandWindow::WaylandWindow(unsigned handle) : shell_surface_(NULL),
    window_(NULL),
    type_(None),
    handle_(handle),
    allocation_(gfx::Rect(0, 0, 1, 1)) {
}

WaylandWindow::~WaylandWindow() {
  wl_surface_set_user_data(GetSurface(), 0);
  delete window_;
  delete shell_surface_;
}

void WaylandWindow::SetShellAttributes(ShellType type) {
  if (type_ == type)
    return;

  if (!shell_surface_) {
    shell_surface_ = new WaylandShellSurface(this);
    wl_surface_set_user_data(GetSurface(), this);
  }

  type_ = type;
  shell_surface_->UpdateShellSurface(type_, NULL, 0, 0);
}

void WaylandWindow::SetShellAttributes(ShellType type,
                                       WaylandShellSurface* shell_parent,
                                       unsigned x,
                                       unsigned y) {
  DCHECK(shell_parent && (type == POPUP));

  if (!shell_surface_) {
    shell_surface_ = new WaylandShellSurface(this);
    wl_surface_set_user_data(GetSurface(), this);
    WaylandInputDevice* input = WaylandDisplay::GetInstance()->PrimaryInput();
    input->SetGrabWindowHandle(handle_, 0);
  }

  type_ = type;
  shell_surface_->UpdateShellSurface(type_, shell_parent, x, y);
}

void WaylandWindow::SetWindowTitle(const base::string16& title) {
  shell_surface_->SetWindowTitle(title);
}

void WaylandWindow::Maximize() {
  NOTIMPLEMENTED();
}

void WaylandWindow::Minimize() {
  NOTIMPLEMENTED();
}

void WaylandWindow::Restore() {
  NOTIMPLEMENTED();
}

void WaylandWindow::ToggleFullscreen() {
  if (type_ == FULLSCREEN)
    SetShellAttributes(TOPLEVEL);
  else
    SetShellAttributes(FULLSCREEN);
}

void WaylandWindow::RealizeAcceleratedWidget() {
  if (!shell_surface_) {
    LOG(ERROR) << "Shell type not set. Setting it to TopLevel";
    SetShellAttributes(TOPLEVEL);
  }

  if (!window_)
    window_ = new EGLWindow(shell_surface_->Surface()->wlSurface(),
                            allocation_.width(),
                            allocation_.height());
}

void WaylandWindow::HandleSwapBuffers() {
  shell_surface_->Surface()->EnsureFrameCallBackDone();
  shell_surface_->Surface()->AddFrameCallBack();
}

wl_egl_window* WaylandWindow::egl_window() const {
  DCHECK(window_);
  return window_->egl_window();
}

struct wl_surface* WaylandWindow::GetSurface() const {
  DCHECK(shell_surface_);
  return shell_surface_->Surface()->wlSurface();
}

bool WaylandWindow::SetBounds(const gfx::Rect& new_bounds) {
  int width = new_bounds.width();
  int height = new_bounds.height();
  allocation_ = gfx::Rect(allocation_.x(), allocation_.y(), width, height);
  if (!shell_surface_ || !window_)
    return false;

  return window_->Resize(shell_surface_->Surface(), width, height);
}

}  // namespace ozonewayland
