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
    case PLATFORM_WINDOW_TYPE_POPUP:
    case PLATFORM_WINDOW_TYPE_MENU: {
      parent_ = parent_window;
      if (!parent_ && window_manager_->GetActiveWindow())
        parent_ = window_manager_->GetActiveWindow()->GetHandle();
      type_ = ui::POPUP;
      ValidateBounds();
      window_manager_->OnRootWindowCreated(this);
      break;
    }
    case PLATFORM_WINDOW_TYPE_TOOLTIP: {
      parent_ = parent_window;
      if (!parent_ && window_manager_->GetActiveWindow())
        parent_ = window_manager_->GetActiveWindow()->GetHandle();
      type_ = ui::TOOLTIP;
      bounds_.set_origin(gfx::Point(0, 0));
      break;
    }
    case PLATFORM_WINDOW_TYPE_BUBBLE:
    case PLATFORM_WINDOW_TYPE_WINDOW:
      parent_ = 0;
      type_ = ui::WINDOW;
      window_manager_->OnRootWindowCreated(this);
      break;
    case PLATFORM_WINDOW_TYPE_WINDOW_FRAMELESS:
      NOTIMPLEMENTED();
      break;
    default:
      break;
  }

  sender_->AddChannelObserver(this);
}

void OzoneWaylandWindow::SetTitle(const base::string16& title) {
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
  clip_region.setRect(0, 0, bounds_.width(), bounds_.height());
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

void OzoneWaylandWindow::RequestDragData(const std::string& mime_type) {
  sender_->Send(new WaylandDisplay_RequestDragData(mime_type));
}

void OzoneWaylandWindow::RequestSelectionData(const std::string& mime_type) {
  sender_->Send(new WaylandDisplay_RequestSelectionData(mime_type));
}

void OzoneWaylandWindow::DragWillBeAccepted(uint32_t serial,
                                            const std::string& mime_type) {
  sender_->Send(new WaylandDisplay_DragWillBeAccepted(serial, mime_type));
}

void OzoneWaylandWindow::DragWillBeRejected(uint32_t serial) {
  sender_->Send(new WaylandDisplay_DragWillBeRejected(serial));
}

gfx::Rect OzoneWaylandWindow::GetBounds() {
  return bounds_;
}

void OzoneWaylandWindow::SetBounds(const gfx::Rect& bounds) {
  int original_x = bounds_.x();
  int original_y = bounds_.y();
  bounds_ = bounds;
  if (type_ == ui::TOOLTIP)
    ValidateBounds();

  if ((original_x != bounds_.x()) || (original_y  != bounds_.y())) {
    sender_->Send(new WaylandDisplay_MoveWindow(handle_, parent_,
                                                type_, bounds_));
  }

  delegate_->OnBoundsChanged(bounds_);
}

void OzoneWaylandWindow::Show() {
  state_ = ui::SHOW;
  SendWidgetState();
}

void OzoneWaylandWindow::Hide() {
  state_ = ui::HIDE;

  if (type_ == ui::TOOLTIP)
    delegate_->OnCloseRequest();
  else
    SendWidgetState();
}

void OzoneWaylandWindow::Close() {
  if (type_ != ui::TOOLTIP)
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
                                          bounds_.x(),
                                          bounds_.y(),
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

void OzoneWaylandWindow::ValidateBounds() {
  DCHECK(parent_);
  gfx::Rect parent_bounds = window_manager_->GetWindow(parent_)->GetBounds();
  int x = bounds_.x() - parent_bounds.x();
  int y = bounds_.y() - parent_bounds.y();

  if (x < parent_bounds.x()) {
    x = parent_bounds.x();
  } else {
    int width = x + bounds_.width();
    if (width > parent_bounds.width())
      x -= width - parent_bounds.width();
  }

  if (y < parent_bounds.y()) {
    y = parent_bounds.y();
  } else {
    int height = y + bounds_.height();
    if (height > parent_bounds.height())
      y -= height - parent_bounds.height();
  }

  bounds_.set_origin(gfx::Point(x, y));
}

PlatformImeController* OzoneWaylandWindow::GetPlatformImeController() {
  return nullptr;
}

}  // namespace ui
