// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/window.h"

#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "ozone/egl/egl_window.h"
#include "ozone/wayland/shell_surface.h"
#include "ozone/wayland/surface.h"
#include "ozone/wayland/global.h"

namespace ui {

WaylandWindow::WaylandWindow()
    : parent_window_(NULL),
    user_data_(NULL),
    window_(NULL),
    state_(Visible),
    type_(TOPLEVEL),
    resize_edges_(0),
    allocation_(gfx::Rect(0, 0, 1, 1)),
    server_allocation_(gfx::Rect(0, 0, 1, 1)),
    saved_allocation_(gfx::Rect(0, 0, 1, 1))
{
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

struct wl_surface* WaylandWindow::wlSurface() const
{
  return shell_surface_->Surface()->wlSurface();
}

wl_egl_window* WaylandWindow::egl_window() const
{
  return window_ ? window_->egl_window() : 0;
}

void WaylandWindow::SetShellType(ShellType type)
{
  if (type_ == type)
    return;

  type_ = type;
}

void WaylandWindow::UpdateWindowType()
{
  if (state_ & FullScreen)
    shell_surface_->UpdateShellSurface(FULLSCREEN);
  else if (!parent_window_)
    shell_surface_->UpdateShellSurface(TOPLEVEL);
  else
     shell_surface_->UpdateShellSurface(TRANSIENT, parent_window_);
}

void WaylandWindow::GetResizeDelta(int &x, int &y)
{
  if (resize_edges_ & WINDOW_RESIZING_LEFT)
    x = server_allocation_.width() - allocation_.width();
  else
    x = 0;

  if (resize_edges_ & WINDOW_RESIZING_TOP)
    y = server_allocation_.height() - allocation_.height();
  else
    y = 0;

  resize_edges_ = 0;
}

void WaylandWindow::AddChild(WaylandWindow* child)
{
  DCHECK(std::find(children_.begin(), children_.end(), child) ==
      children_.end());
  if (child->GetParentWindow())
    child->GetParentWindow()->RemoveChild(child);
  child->parent_window_ = this;
  children_.push_back(child);
}

void WaylandWindow::RemoveChild(WaylandWindow* child)
{
  Windows::iterator i = std::find(children_.begin(), children_.end(), child);
  DCHECK(i != children_.end());
  child->parent_window_ = NULL;
  children_.erase(i);
}

void WaylandWindow::SetParentWindow(WaylandWindow* parent_window)
{
  if(parent_window)
    parent_window->AddChild(this);

    UpdateWindowType();
}

void WaylandWindow::SetBounds(const gfx::Rect& new_bounds)
{
  if((type_ & TRANSIENT) && parent_window_)
  {
    gfx::Point relativeposition = new_bounds.origin();
      if ((relativeposition.x() != allocation_.x()) || (relativeposition.y() != allocation_.y())) {
        allocation_.set_origin(new_bounds.origin());
        shell_surface_->UpdateShellSurface(type_, parent_window_);
      }
  }

  HandleResize(new_bounds.width(), new_bounds.height());
}

void WaylandWindow::SetWindowTitle(const char *title)
{
    shell_surface_->SetWindowTitle(title);
}

void WaylandWindow::OnShow()
{
  if (state_ & Visible)
    return;

  state_ |= Visible;
  if (state_ & PendingResize)
      Resize();

  // (kalyan) Handle Resource Allocation.
}

void WaylandWindow::OnHide()
{
  if (!(state_ & Visible))
    return;

  state_ &= ~Visible;
  // (kalyan) Release all resources here.
}

void WaylandWindow::OnActivate()
{
  if (state_ & Activated)
    return;

  state_ |= Activated;
  //(kalyan) Handle focus here.
}

void WaylandWindow::OnDeActivate()
{
  if (!(state_ & Activated))
    return;

  state_ &= ~Activated;
  // (kalyan) reset focus.
}

void WaylandWindow::OnMaximize()
{
  if (state_ & Maximized)
    return;

  state_ |= Maximized;
  state_ &= ~Minimized;
  shell_surface_->SetMaximized(type_);
  OnShow();
}

void WaylandWindow::OnMinimize()
{
  if (state_ & Minimized)
    return;

  state_ &= ~Maximized;
  state_ |= Minimized;
  OnHide();
  shell_surface_->SetMinimized();
}

void WaylandWindow::OnRestore()
{
  if (state_ & Normal)
    return;

  state_ &= ~Maximized;
  state_ &= ~Minimized;
  state_ |= Normal;
  UpdateWindowType();
  OnShow();
}

void WaylandWindow::HandleResize(int32_t width, int32_t height)
{
  if ((width == allocation_.width()) && (allocation_.height() == height))
      return;

  allocation_ = gfx::Rect(allocation_.x(), allocation_.y(), width, height);
  if (!(state_ & Visible)) {
      state_ |= PendingResize;
      return;
  }

  Resize();
}

void WaylandWindow::Resize()
{
  if (!window_)
    return;

  window_->Resize(allocation_.width(), allocation_.height());
  state_ &= ~PendingResize;
}

void WaylandWindow::SetFocus(bool focus)
{
  if (focus)
    state_ |= Focus;
  else
    state_ &= ~Focus;
}

void WaylandWindow::SetFullScreen(bool fullscreen)
{
  if ((state_ & FullScreen) == fullscreen)
    return;

  if (fullscreen)
    state_ |= FullScreen;
  else
    state_ &= ~FullScreen;

  UpdateWindowType();
}

void WaylandWindow::RealizeAcceleratedWidget()
{
  if (!window_) {
      window_ = new EGLWindow(shell_surface_->Surface()->wlSurface(), allocation_.width(), allocation_.height());
    state_ &= ~PendingResize;
  }
}

void WaylandWindow::HandleConfigure(uint32_t edges, int32_t width, int32_t height)
{
  if (width <= 0 || height <= 0)
    return;

  resize_edges_ = edges;
  HandleResize(width, height);
}

}  // namespace ui
