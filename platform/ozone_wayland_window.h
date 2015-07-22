// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_PLATFORM_OZONE_WAYLAND_WINDOW_H_
#define OZONE_PLATFORM_OZONE_WAYLAND_WINDOW_H_

#include "ozone/ui/events/window_constants.h"
#include "ui/events/platform/platform_event_dispatcher.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/ozone/platform/drm/host/channel_observer.h"
#include "ui/platform_window/platform_window.h"

namespace ui {

class OzoneGpuPlatformSupportHost;
class PlatformWindowDelegate;
class WindowManagerWayland;

class OzoneWaylandWindow : public PlatformWindow,
                           public PlatformEventDispatcher,
                           public ChannelObserver {
 public:
  OzoneWaylandWindow(PlatformWindowDelegate* delegate,
                     OzoneGpuPlatformSupportHost* sender,
                     WindowManagerWayland* window_manager,
                     const gfx::Rect& bounds);
  ~OzoneWaylandWindow() override;

  unsigned GetHandle() const { return handle_; }
  PlatformWindowDelegate* GetDelegate() const { return delegate_; }

  // PlatformWindow:
  void InitPlatformWindow(PlatformWindowType type,
                          gfx::AcceleratedWidget parent_window) override;
  void SetWidgetCursor(int cursor_type) override;
  void SetWidgetTitle(const base::string16& title) override;
  gfx::Rect GetBounds() override;
  void SetBounds(const gfx::Rect& bounds) override;
  void Show() override;
  void Hide() override;
  void Close() override;
  void SetCapture() override;
  void ReleaseCapture() override;
  void ToggleFullscreen() override;
  void Maximize() override;
  void Minimize() override;
  void Restore() override;
  void SetCursor(PlatformCursor cursor) override;
  void MoveCursorTo(const gfx::Point& location) override;
  void ConfineCursorToBounds(const gfx::Rect& bounds) override;

  // PlatformEventDispatcher:
  bool CanDispatchEvent(const PlatformEvent& event) override;
  uint32_t DispatchEvent(const PlatformEvent& event) override;

  // ChannelObserver:
  void OnChannelEstablished() override;
  void OnChannelDestroyed() override;

 private:
  void SendWidgetState();
  PlatformWindowDelegate* delegate_;   // Not owned.
  OzoneGpuPlatformSupportHost* sender_;  // Not owned.
  WindowManagerWayland* window_manager_;  // Not owned.
  gfx::Rect bounds_;
  unsigned handle_;
  unsigned parent_;
  gfx::Point pos_;
  ui::WidgetType type_;
  ui::WidgetState state_;
  int cursor_type_;
  base::string16 title_;

  DISALLOW_COPY_AND_ASSIGN(OzoneWaylandWindow);
};

}  // namespace ui

#endif  // OZONE_PLATFORM_OZONE_WAYLAND_WINDOW_H_
