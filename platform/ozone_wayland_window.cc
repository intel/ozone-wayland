// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/platform/ozone_wayland_window.h"

#include "ozone/ui/desktop_aura/desktop_screen_wayland.h"
#include "ozone/ui/events/window_state_change_handler.h"
#include "ui/platform_window/platform_window_delegate.h"

namespace ui {

OzoneWaylandWindow::OzoneWaylandWindow(PlatformWindowDelegate* delegate,
                                       const gfx::Rect& bounds)
    : delegate_(delegate), bounds_(bounds) {
  static int opaque_handle = 0;
  opaque_handle++;
  handle_ = opaque_handle;
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(handle_,
                                                              CREATE,
                                                              0,
                                                              0);
  delegate_->OnAcceleratedWidgetAvailable(handle_);
}

OzoneWaylandWindow::~OzoneWaylandWindow() {
}

gfx::Rect OzoneWaylandWindow::GetBounds() {
  return bounds_;
}

void OzoneWaylandWindow::SetBounds(const gfx::Rect& bounds) {
  gfx::Rect previous_bounds = bounds_;
  bounds_ = bounds;
  delegate_->OnBoundChanged(previous_bounds, bounds_);
}

void OzoneWaylandWindow::Show() {
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(handle_,
                                                              ui::SHOW);
}

void OzoneWaylandWindow::Hide() {
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(handle_,
                                                              ui::HIDE);
}

void OzoneWaylandWindow::Close() {
}

void OzoneWaylandWindow::SetCapture() {
}

void OzoneWaylandWindow::ReleaseCapture() {
}

void OzoneWaylandWindow::ToggleFullscreen() {
  gfx::Screen *screen = gfx::Screen::GetScreenByType(gfx::SCREEN_TYPE_NATIVE);
  if (!screen)
    NOTREACHED() << "Unable to retrieve valid gfx::Screen";

  SetBounds(screen->GetPrimaryDisplay().bounds());
  // We could use HandleConfigure in ShellSurface to set the correct bounds of
  // egl window associated with this opaque handle. How ever, this would need
  // to handle race conditions and ensure correct size is set for
  // wl_egl_window_resize before eglsurface is resized. Passing window size
  // attributes already here, ensures that wl_egl_window_resize is resized
  // before eglsurface is resized. This doesn't add any extra overhead as the
  // IPC call needs to be done.
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(handle_,
                                                              ui::FULLSCREEN,
                                                              bounds_.width(),
                                                              bounds_.height());
}

void OzoneWaylandWindow::Maximize() {
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(handle_,
                                                              ui::MAXIMIZED);
}

void OzoneWaylandWindow::Minimize() {
  SetBounds(gfx::Rect());
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(handle_,
                                                              ui::MINIMIZED);
}

void OzoneWaylandWindow::Restore() {
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(handle_,
                                                              ui::RESTORE,
                                                              bounds_.width(),
                                                              bounds_.height());
}

void OzoneWaylandWindow::SetCursor(PlatformCursor cursor) {
}

void OzoneWaylandWindow::MoveCursorTo(const gfx::Point& location) {
}

}  // namespace ui
