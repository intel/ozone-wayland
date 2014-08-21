// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/platform/ozone_wayland_window.h"

#include "ozone/ui/events/window_state_change_handler.h"
#include "ui/platform_window/platform_window_delegate.h"

namespace ui {

OzoneWaylandWindow::OzoneWaylandWindow(PlatformWindowDelegate* delegate,
                                       const gfx::Rect& bounds)
    : delegate_(delegate), bounds_(bounds) {
  static int opaque_handle = 0;
  opaque_handle++;
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(opaque_handle,
                                                              CREATE,
                                                              0,
                                                              0);
  delegate_->OnAcceleratedWidgetAvailable(opaque_handle);
}

OzoneWaylandWindow::~OzoneWaylandWindow() {
}

gfx::Rect OzoneWaylandWindow::GetBounds() {
  return bounds_;
}

void OzoneWaylandWindow::SetBounds(const gfx::Rect& bounds) {
  bounds_ = bounds;
  delegate_->OnBoundsChanged(bounds);
}

void OzoneWaylandWindow::Show() {
}

void OzoneWaylandWindow::Hide() {
}

void OzoneWaylandWindow::Close() {
}

void OzoneWaylandWindow::SetCapture() {
}

void OzoneWaylandWindow::ReleaseCapture() {
}

void OzoneWaylandWindow::ToggleFullscreen() {
}

void OzoneWaylandWindow::Maximize() {
}

void OzoneWaylandWindow::Minimize() {
}

void OzoneWaylandWindow::Restore() {
}

void OzoneWaylandWindow::SetCursor(PlatformCursor cursor) {
}

void OzoneWaylandWindow::MoveCursorTo(const gfx::Point& location) {
}

}  // namespace ui
