// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/platform/ozone_wayland_window.h"

#include <vector>
#include "base/bind.h"
#include "ozone/platform/messages.h"
#include "ozone/platform/ozone_gpu_platform_support_host.h"
#include "ozone/platform/window_manager_wayland.h"
#include "ui/base/cursor/ozone/bitmap_cursor_factory_ozone.h"
#include "ui/events/ozone/events_ozone.h"
#include "ui/events/platform/platform_event_source.h"
#include "ui/gfx/screen.h"
#include "ui/platform_window/platform_window_delegate.h"

namespace ui {

OzoneWaylandWindow::OzoneWaylandWindow(PlatformWindowDelegate* delegate,
                                       OzoneGpuPlatformSupportHost* sender,
                                       WindowManagerWayland* window_manager,
                                       const gfx::Rect& bounds)
    : delegate_(delegate),
      sender_(sender),
      window_manager_(window_manager),
      transparent_(false),
      bounds_(bounds),
      parent_(0),
      state_(UNINITIALIZED),
      region_(NULL),
      cursor_type_(-1) {
  static int opaque_handle = 0;
  opaque_handle++;
  handle_ = opaque_handle;
  delegate_->OnAcceleratedWidgetAvailable(opaque_handle, 1.0);
  window_manager_->OnRootWindowCreated(this);
}

OzoneWaylandWindow::~OzoneWaylandWindow() {
  sender_->RemoveChannelObserver(this);
  PlatformEventSource::GetInstance()->RemovePlatformEventDispatcher(this);
  if (region_)
    delete region_;
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
          parent_window ? window_manager_->GetWindow(parent_window)
                        : window_manager_->GetActiveWindow();

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

void OzoneWaylandWindow::SetWidgetTitle(const base::string16& title) {
  title_ = title;
  if (!sender_->IsConnected())
    return;

  sender_->Send(new WaylandDisplay_Title(handle_, title_));
}

void OzoneWaylandWindow::SetWindowShape(const SkPath& path) {
  ResetRegion();
  if (transparent_)
    return;

  region_ = new SkRegion();
  SkRegion clip_region;
  clip_region.setRect(pos_.x(), pos_.y(), bounds_.width(), bounds_.height());
  region_->setPath(path, clip_region);
  AddRegion();
}

void OzoneWaylandWindow::SetOpacity(unsigned char opacity) {
  if (opacity == 255) {
    if (transparent_) {
      AddRegion();
      transparent_ = false;
    }
  } else if (!transparent_) {
    ResetRegion();
    transparent_ = true;
  }
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
  window_manager_->OnRootWindowClosed(this);
}

void OzoneWaylandWindow::SetCapture() {
  window_manager_->GrabEvents(handle_);
}

void OzoneWaylandWindow::ReleaseCapture() {
  window_manager_->UngrabEvents(handle_);
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
  window_manager_->Restore(this);
  state_ = ui::RESTORE;
  SendWidgetState();
}

void OzoneWaylandWindow::SetCursor(PlatformCursor cursor) {
  if (window_manager_->GetPlatformCursor() == cursor)
    return;

  scoped_refptr<BitmapCursorOzone> bitmap =
      BitmapCursorFactoryOzone::GetBitmapCursor(cursor);
  bitmap_ = bitmap;
  window_manager_->SetPlatformCursor(cursor);
  if (!sender_->IsConnected())
    return;

  SetCursor();
}

void OzoneWaylandWindow::MoveCursorTo(const gfx::Point& location) {
  sender_->Send(new WaylandDisplay_MoveCursor(location));
}

void OzoneWaylandWindow::ConfineCursorToBounds(const gfx::Rect& bounds) {
}

////////////////////////////////////////////////////////////////////////////////
// WindowTreeHostDelegateWayland, ui::PlatformEventDispatcher implementation:
bool OzoneWaylandWindow::CanDispatchEvent(
    const ui::PlatformEvent& ne) {
  return window_manager_->event_grabber() == handle_;
}

uint32_t OzoneWaylandWindow::DispatchEvent(
    const ui::PlatformEvent& ne) {
  DispatchEventFromNativeUiEvent(
      ne, base::Bind(&PlatformWindowDelegate::DispatchEvent,
                     base::Unretained(delegate_)));
  return POST_DISPATCH_STOP_PROPAGATION;
}

void OzoneWaylandWindow::OnChannelEstablished() {
  sender_->Send(new WaylandDisplay_Create(handle_,
                                          parent_,
                                          pos_.x(),
                                          pos_.y(),
                                          type_));
  if (state_)
    sender_->Send(new WaylandDisplay_State(handle_, state_));

  if (title_.length())
    sender_->Send(new WaylandDisplay_Title(handle_, title_));

  AddRegion();
  SetCursor();
}

void OzoneWaylandWindow::OnChannelDestroyed() {
}

void OzoneWaylandWindow::SendWidgetState() {
  if (!sender_->IsConnected())
    return;

  sender_->Send(new WaylandDisplay_State(handle_, state_));
}

void OzoneWaylandWindow::AddRegion() {
  if (sender_->IsConnected() && region_ && !region_->isEmpty()) {
     const SkIRect& rect = region_->getBounds();
     sender_->Send(new WaylandDisplay_AddRegion(handle_,
                                                rect.left(),
                                                rect.top(),
                                                rect.right(),
                                                rect.bottom()));
  }
}

void OzoneWaylandWindow::ResetRegion() {
  if (region_) {
    if (sender_->IsConnected() && !region_->isEmpty()) {
      const SkIRect& rect = region_->getBounds();
      sender_->Send(new WaylandDisplay_SubRegion(handle_,
                                                 rect.left(),
                                                 rect.top(),
                                                 rect.right(),
                                                 rect.bottom()));
    }

    delete region_;
    region_ = NULL;
  }
}

void OzoneWaylandWindow::SetCursor() {
  if (bitmap_) {
    sender_->Send(new WaylandDisplay_CursorSet(bitmap_->bitmaps(),
                                               bitmap_->hotspot()));
  } else {
    sender_->Send(new WaylandDisplay_CursorSet(std::vector<SkBitmap>(),
                                               gfx::Point()));
  }
}

}  // namespace ui
