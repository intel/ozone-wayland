// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/platform/ozone_wayland_window.h"

#include "ozone/ui/desktop_aura/desktop_screen_wayland.h"
#include "ozone/ui/desktop_aura/desktop_window_tree_host_wayland.h"
#include "ozone/ui/desktop_aura/window_tree_host_delegate_wayland.h"
#include "ozone/ui/events/window_state_change_handler.h"
#include "ui/platform_window/platform_window_delegate.h"

namespace ui {

views::WindowTreeHostDelegateWayland*
    OzoneWaylandWindow::g_delegate_ozone_wayland_ = NULL;

OzoneWaylandWindow::OzoneWaylandWindow(PlatformWindowDelegate* delegate,
                                       const gfx::Rect& bounds)
    : delegate_(delegate), bounds_(bounds) {
  static int opaque_handle = 0;
  opaque_handle++;
  handle_ = opaque_handle;
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(handle_,
                                                              CREATE);
  delegate_->OnAcceleratedWidgetAvailable(opaque_handle);

  if (!g_delegate_ozone_wayland_)
    g_delegate_ozone_wayland_ = new views::WindowTreeHostDelegateWayland();

  g_delegate_ozone_wayland_->OnRootWindowCreated(this);
}

OzoneWaylandWindow::~OzoneWaylandWindow() {
}

void OzoneWaylandWindow::InitPlatformWindow(PlatformWindowType type) {
  ui::WindowStateChangeHandler* state_handler =
      ui::WindowStateChangeHandler::GetInstance();
  switch (type) {
    case PLATFORM_WINDOW_TYPE_TOOLTIP:
    case PLATFORM_WINDOW_TYPE_POPUP:
    case PLATFORM_WINDOW_TYPE_MENU: {
      // Wayland surfaces don't know their position on the screen and transient
      // surfaces always require a parent surface for relative placement. Here
      // there's a catch because content_shell menus don't have parent and
      // therefore we use root window to calculate their position.
      OzoneWaylandWindow* active_window =
          g_delegate_ozone_wayland_->GetActiveWindow();
      DCHECK(active_window);
      const gfx::Rect& parent_bounds = active_window->GetBounds();
      // Transient type expects a position relative to the parent
      gfx::Point transientPos(bounds_.x() - parent_bounds.x(),
                              bounds_.y() - parent_bounds.y());
      // Different platforms implement different input grab pointer behaviors
      // on Chromium. While the Linux GTK+ grab button clicks but not the
      // pointer movement, the MS Windows implementation don't implement any
      // pointer grab. In here we're using another different behavior for
      // Chromium, but which is the common sense on most Wayland UI
      // environments, where the input pointer is grabbed as a whole when a
      // menu type of window is opened. I.e. both pointer clicks and movements
      // will be routed only to the newly created window (grab installed). For
      // more information please refer to the Wayland protocol.
      state_handler->SetWidgetAttributes(handle_,
                                         active_window->GetHandle(),
                                         transientPos.x(),
                                         transientPos.y(),
                                         ui::POPUP);
      break;
    }
    case PLATFORM_WINDOW_TYPE_BUBBLE:
    case PLATFORM_WINDOW_TYPE_WINDOW:
      state_handler->SetWidgetAttributes(handle_,
                                         0,
                                         0,
                                         0,
                                         ui::WINDOW);
      break;
    case PLATFORM_WINDOW_TYPE_WINDOW_FRAMELESS:
      NOTIMPLEMENTED();
      break;
    default:
      break;
  }
}

void OzoneWaylandWindow::Activate() {
  g_delegate_ozone_wayland_->SetActiveWindow(this);
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(handle_,
                                                              ui::ACTIVE);
}

void OzoneWaylandWindow::DeActivate()  {
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(handle_,
                                                              ui::INACTIVE);
  g_delegate_ozone_wayland_->DeActivateWindow(this);

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
  g_delegate_ozone_wayland_->OnRootWindowClosed(this);
  if (!g_delegate_ozone_wayland_->GetActiveWindowHandle()) {
    // We have no open windows, free g_delegate_ozone_wayland_.
    delete g_delegate_ozone_wayland_;
    g_delegate_ozone_wayland_ = NULL;
  }
}

void OzoneWaylandWindow::SetCapture() {
  g_delegate_ozone_wayland_->SetCapture(handle_);
}

void OzoneWaylandWindow::ReleaseCapture() {
  g_delegate_ozone_wayland_->SetCapture(0);
}

void OzoneWaylandWindow::ToggleFullscreen() {
  gfx::Screen *screen = gfx::Screen::GetScreenByType(gfx::SCREEN_TYPE_NATIVE);
  if (!screen)
    NOTREACHED() << "Unable to retrieve valid gfx::Screen";

  SetBounds(screen->GetPrimaryDisplay().bounds());
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(handle_,
                                                              ui::FULLSCREEN);
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
                                                              ui::RESTORE);
}

void OzoneWaylandWindow::SetCursor(PlatformCursor cursor) {
}

void OzoneWaylandWindow::MoveCursorTo(const gfx::Point& location) {
}

}  // namespace ui
