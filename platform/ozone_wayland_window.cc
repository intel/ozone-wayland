// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/platform/ozone_wayland_window.h"

#include "base/bind.h"
#include "ozone/platform/ozone_gpu_platform_support_host.h"
#include "ozone/platform/window_manager_wayland.h"
#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ozone/ui/public/messages.h"
#include "ui/events/ozone/events_ozone.h"
#include "ui/events/platform/platform_event_source.h"
#include "ui/gfx/screen.h"
#include "ui/platform_window/platform_window_delegate.h"

namespace ui {

WindowManagerWayland*
    OzoneWaylandWindow::g_delegate_ozone_wayland_ = NULL;

OzoneWaylandWindow::OzoneWaylandWindow(PlatformWindowDelegate* delegate,
                                       OzoneGpuPlatformSupportHost* sender,
                                       const gfx::Rect& bounds)
    : delegate_(delegate),
      sender_(sender),
      bounds_(bounds),
      parent_(0),
      state_(UNINITIALIZED),
      cursor_type_(-1) {
  static int opaque_handle = 0;
  opaque_handle++;
  handle_ = opaque_handle;
  delegate_->OnAcceleratedWidgetAvailable(opaque_handle, 1.0);

  if (!g_delegate_ozone_wayland_)
    g_delegate_ozone_wayland_ = new WindowManagerWayland();

  g_delegate_ozone_wayland_->OnRootWindowCreated(this);
}

OzoneWaylandWindow::~OzoneWaylandWindow() {
  sender_->RemoveChannelObserver(this);
  PlatformEventSource::GetInstance()->RemovePlatformEventDispatcher(this);
}

void OzoneWaylandWindow::InitPlatformWindow(
    PlatformWindowType type, gfx::AcceleratedWidget parent_window) {
  PlatformEventSource::GetInstance()->AddPlatformEventDispatcher(this);
  switch (type) {
    case PLATFORM_WINDOW_TYPE_TOOLTIP:
    case PLATFORM_WINDOW_TYPE_POPUP:
    case PLATFORM_WINDOW_TYPE_MENU: {
      // Wayland surfaces don't know their position on the screen and transient
      // surfaces always require a parent surface for relative placement. Here
      // there's a catch because content_shell menus don't have parent and
      // therefore we use root window to calculate their position.
      OzoneWaylandWindow* active_window =
          parent_window ? g_delegate_ozone_wayland_->GetWindow(parent_window)
                        : g_delegate_ozone_wayland_->GetActiveWindow();

      DCHECK(active_window);
      gfx::Rect parent_bounds = active_window->GetBounds();
      parent_ = active_window->GetHandle();
      type_ = ui::POPUP;
      // Don't size the window bigger than the parent, otherwise the user may
      // not be able to close or move it.
      // Transient type expects a position relative to the parent
      pos_ = gfx::Point(bounds_.x() - parent_bounds.x(),
                        bounds_.y() - parent_bounds.y());
      break;
    }
    case PLATFORM_WINDOW_TYPE_BUBBLE:
    case PLATFORM_WINDOW_TYPE_WINDOW:
      parent_ = 0;
      pos_ = gfx::Point(0, 0);
      type_ = ui::WINDOW;
      break;
    case PLATFORM_WINDOW_TYPE_WINDOW_FRAMELESS:
      NOTIMPLEMENTED();
      break;
    default:
      break;
  }

  sender_->AddChannelObserver(this);
}

void OzoneWaylandWindow::SetWidgetCursor(int cursor_type) {
  cursor_type_ = cursor_type;
  if (!sender_->IsConnected())
    return;

  sender_->Send(new WaylandWindow_Cursor(cursor_type_));
}

void OzoneWaylandWindow::SetWidgetTitle(const base::string16& title) {
  title_ = title;
  if (!sender_->IsConnected())
    return;

  sender_->Send(new WaylandWindow_Title(handle_, title_));
}

gfx::Rect OzoneWaylandWindow::GetBounds() {
  return bounds_;
}

void OzoneWaylandWindow::SetBounds(const gfx::Rect& bounds) {
  bounds_ = bounds;
  delegate_->OnBoundsChanged(bounds_);
}

void OzoneWaylandWindow::Show() {
  state_ = ui::SHOW;
  SendWidgetState();
}

void OzoneWaylandWindow::Hide() {
  state_ = ui::HIDE;
  SendWidgetState();
}

void OzoneWaylandWindow::Close() {
  g_delegate_ozone_wayland_->OnRootWindowClosed(this);
  if (!g_delegate_ozone_wayland_->HasWindowsOpen()) {
    // We have no open windows, free g_delegate_ozone_wayland_.
    delete g_delegate_ozone_wayland_;
    g_delegate_ozone_wayland_ = NULL;
  }
}

void OzoneWaylandWindow::SetCapture() {
  g_delegate_ozone_wayland_->GrabEvents(handle_);
}

void OzoneWaylandWindow::ReleaseCapture() {
  g_delegate_ozone_wayland_->UngrabEvents(handle_);
}

void OzoneWaylandWindow::ToggleFullscreen() {
  gfx::Screen *screen = gfx::Screen::GetScreenByType(gfx::SCREEN_TYPE_NATIVE);
  if (!screen)
    NOTREACHED() << "Unable to retrieve valid gfx::Screen";

  SetBounds(screen->GetPrimaryDisplay().bounds());
  state_ = ui::FULLSCREEN;
  SendWidgetState();
}

void OzoneWaylandWindow::Maximize() {
  state_ = ui::MAXIMIZED;
  SendWidgetState();
}

void OzoneWaylandWindow::Minimize() {
  SetBounds(gfx::Rect());
  state_ = ui::MINIMIZED;
  SendWidgetState();
}

void OzoneWaylandWindow::Restore() {
  g_delegate_ozone_wayland_->Restore(this);
  state_ = ui::RESTORE;
  SendWidgetState();
}

void OzoneWaylandWindow::SetCursor(PlatformCursor cursor) {
}

void OzoneWaylandWindow::MoveCursorTo(const gfx::Point& location) {
}

void OzoneWaylandWindow::ConfineCursorToBounds(const gfx::Rect& bounds) {
}

////////////////////////////////////////////////////////////////////////////////
// WindowTreeHostDelegateWayland, ui::PlatformEventDispatcher implementation:
bool OzoneWaylandWindow::CanDispatchEvent(
    const ui::PlatformEvent& ne) {
  return g_delegate_ozone_wayland_->event_grabber() == handle_;
}

uint32_t OzoneWaylandWindow::DispatchEvent(
    const ui::PlatformEvent& ne) {
  DispatchEventFromNativeUiEvent(
      ne, base::Bind(&PlatformWindowDelegate::DispatchEvent,
                     base::Unretained(delegate_)));
  return POST_DISPATCH_STOP_PROPAGATION;
}

void OzoneWaylandWindow::OnChannelEstablished() {
  sender_->Send(new WaylandWindow_Create(handle_,
                                         parent_,
                                         pos_.x(),
                                         pos_.y(),
                                         type_));
  if (state_)
    sender_->Send(new WaylandWindow_State(handle_, state_));

  if (title_.length())
    sender_->Send(new WaylandWindow_Title(handle_, title_));

  if (cursor_type_ >= 0)
    sender_->Send(new WaylandWindow_Cursor(cursor_type_));
}

void OzoneWaylandWindow::OnChannelDestroyed() {
}

void OzoneWaylandWindow::SendWidgetState() {
  if (!sender_->IsConnected())
    return;

  sender_->Send(new WaylandWindow_State(handle_, state_));
}

}  // namespace ui
