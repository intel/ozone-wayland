// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/window.h"

#include "base/logging.h"
#include "ozone/wayland/egl/egl_window.h"
#include "ozone/wayland/surface.h"
#include "ozone/wayland/task.h"
#include "ozone/wayland/input_device.h"
#include "ui/gl/gl_surface.h"

#include <algorithm>

namespace ui {

WaylandWindow::WaylandWindow()
    : parent_window_(NULL),
    user_data_(NULL),
    relative_position_(),
    surface_(NULL),
    shell_surface_(NULL),
    fullscreen_(false),
    window_(NULL),
    type_(TYPE_TOPLEVEL),
    resize_edges_(0),
    allocation_(gfx::Rect(0, 0, 0, 0)),
    server_allocation_(gfx::Rect(0, 0, 0, 0)),
    saved_allocation_(gfx::Rect(0, 0, 0, 0))
{
  WaylandDisplay* display = WaylandDisplay::GetDisplay();
  if (!display)
      return;

  surface_ = new WaylandSurface();
  if (display->shell())
  {
    shell_surface_ = wl_shell_get_shell_surface(display->shell(), surface_->wlSurface());
  }

  wl_surface_set_user_data(surface_->wlSurface(), this);

  if (shell_surface_)
  {
    wl_shell_surface_set_user_data(shell_surface_, this);

    static const wl_shell_surface_listener shell_surface_listener = {
      WaylandWindow::HandlePing,
      WaylandWindow::HandleConfigure,
      WaylandWindow::HandlePopupDone
    };

    wl_shell_surface_add_listener(shell_surface_,
        &shell_surface_listener, this);
  }

  allocation_.SetRect(0, 0, 0, 0);
  saved_allocation_ = allocation_;

  display->AddWindow(this);
}

wl_egl_window* WaylandWindow::egl_window() const
{
  return window_ ? window_->egl_window() : 0;
}

void WaylandWindow::SetType()
{
  if (!shell_surface_)
    return;

  switch (type_) {
    case TYPE_FULLSCREEN:
      wl_shell_surface_set_fullscreen(shell_surface_,
          WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT, 0, NULL);
      break;
    case TYPE_TOPLEVEL:
      wl_shell_surface_set_toplevel(shell_surface_);
      break;
    case TYPE_TRANSIENT:
      wl_shell_surface_set_transient(shell_surface_,
          parent_window_->surface_->wlSurface(),
          relative_position_.x(), relative_position_.y(), 0);
      break;
    case TYPE_MENU:
      break;
    case TYPE_CUSTOM:
      break;
  }
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
  if (fullscreen_) {
    type_ = TYPE_FULLSCREEN;
  } else if (!parent_window) {
    type_ = TYPE_TOPLEVEL;
  } else {
    type_ = TYPE_TRANSIENT;
  }

  if(parent_window)
    parent_window->AddChild(this);

    SetType();
}

gfx::Rect WaylandWindow::GetBounds() const
{
  gfx::Rect rect = allocation_;

  if(type_ == TYPE_TRANSIENT && parent_window_)
    rect.set_origin(relative_position_);

  return rect;
}

void WaylandWindow::SetBounds(const gfx::Rect& new_bounds)
{
  if(type_ == TYPE_TRANSIENT && parent_window_ &&
      new_bounds.origin() != relative_position_)
  {
    relative_position_ = new_bounds.origin();
  }

  HandleResize(new_bounds.width(), new_bounds.height());
}

void WaylandWindow::Show()
{
  NOTIMPLEMENTED();
}

void WaylandWindow::Hide()
{
  NOTIMPLEMENTED();
}

WaylandWindow::~WaylandWindow() {
  if (window_) {
    delete window_;
    window_ = NULL;
  }

  if (surface_)
  {
    delete surface_;
    surface_ = NULL;
  }

  WaylandDisplay::GetDisplay()->RemoveWindow(this);
}

bool WaylandWindow::IsVisible() const {
  return surface_ != NULL;
}

void WaylandWindow::HandleResize(int32_t width, int32_t height)
{
  if ((width == allocation_.width()) && (allocation_.height() == height))
      return;

  allocation_ = gfx::Rect(allocation_.x(), allocation_.y(), width, height);
  Resize();
}

void WaylandWindow::Resize()
{
  if (!window_) {
    RealizeAcceleratedWidget();
    return;
  }

  window_->Resize(allocation_.width(), allocation_.height());
}

void WaylandWindow::RealizeAcceleratedWidget()
{
  if (!window_)
      window_ = new EGLWindow(surface_->wlSurface(), allocation_.width(),
                              allocation_.height());
}

void WaylandWindow::ScheduleFlush()
{
    WaylandDisplay::GetDisplay()->addPendingTask();
}

void WaylandWindow::SchedulePaintInRect(const gfx::Rect& rect)
{
  ScheduleFlush();
}

void WaylandWindow::HandleConfigure(void *data, struct wl_shell_surface *shell_surface,
    uint32_t edges, int32_t width, int32_t height)
{
  WaylandWindow *window = static_cast<WaylandWindow*>(data);

  if (width <= 0 || height <= 0)
    return;

  window->resize_edges_ = edges;
  window->HandleResize(width, height);
}

void WaylandWindow::HandlePopupDone(void *data, struct wl_shell_surface *shell_surface)
{
}

void WaylandWindow::HandlePing(void *data, struct wl_shell_surface *shell_surface, uint32_t serial)
{
  wl_shell_surface_pong(shell_surface, serial);
}

}  // namespace ui
