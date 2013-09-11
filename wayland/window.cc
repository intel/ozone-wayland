// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/window.h"

#include "ozone/wayland/egl/egl_window.h"
#include "ozone/wayland/shell_surface.h"
#include "ozone/wayland/surface.h"

#include "base/logging.h"

namespace ui {

WaylandWindow::WaylandWindow(ShellType type)
    : shell_surface_(NULL),
    window_(NULL),
    type_(type),
    allocation_(gfx::Rect(0, 0, 1, 1)),
    id_(0)
{
  static WaylandWindowId bufferHandleId = 0;
  bufferHandleId++;
  id_ = bufferHandleId;

  if (type_ != None)
    shell_surface_ = new WaylandShellSurface(this);
}

WaylandWindow::~WaylandWindow() {
  if (window_) {
    delete window_;
    window_ = NULL;
  }

  if (shell_surface_)
  {
    delete shell_surface_;
    shell_surface_ = NULL;
  }
}

wl_egl_window* WaylandWindow::egl_window() const
{
  return window_ ? window_->egl_window() : 0;
}

void WaylandWindow::SetShellType(ShellType type)
{
  if (!shell_surface_ || (type_ == type))
    return;

  type_ = type;
  switch (type_) {
    case TOPLEVEL:
      shell_surface_->UpdateShellSurface(TOPLEVEL);
      break;
    case FULLSCREEN:
    case TRANSIENT:
    case MENU:
    case CUSTOM:
      NOTREACHED() << "UnSupported Shell Type.";
      break;
    default:
      break;
  }
}

bool WaylandWindow::SetBounds(const gfx::Rect& new_bounds)
{
  int width = new_bounds.width();
  int height = new_bounds.height();
  allocation_ = gfx::Rect(allocation_.x(), allocation_.y(), width, height);
  if (!shell_surface_ || !window_)
      return false;

  return window_->Resize(shell_surface_->Surface(), width, height);
}

void WaylandWindow::RealizeAcceleratedWidget()
{
  if (!window_)
    window_ = new EGLWindow(shell_surface_->Surface()->wlSurface(),
                            allocation_.width(), allocation_.height());
}

void WaylandWindow::HandleSwapBuffers()
{
  shell_surface_->Surface()->ensureFrameCallBackDone();
  shell_surface_->Surface()->addFrameCallBack();
}

}  // namespace ui
