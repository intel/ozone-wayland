// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_PLATFORM_OZONE_WAYLAND_WINDOW_H_
#define OZONE_PLATFORM_OZONE_WAYLAND_WINDOW_H_

#include <string>
#include "base/memory/ref_counted.h"
#include "ozone/platform/window_constants.h"
#include "third_party/skia/include/core/SkRegion.h"
#include "ui/events/platform/platform_event_dispatcher.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/ozone/platform/drm/host/channel_observer.h"
#include "ui/platform_window/platform_window.h"

namespace ui {

class BitmapCursorOzone;
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
  void SetTitle(const base::string16& title) override;
  void SetWindowShape(const SkPath& path) override;
  void SetOpacity(unsigned char opacity) override;
  void RequestDragData(const std::string& mime_type) override;
  void RequestSelectionData(const std::string& mime_type) override;
  void DragWillBeAccepted(uint32_t serial,
                          const std::string& mime_type) override;
  void DragWillBeRejected(uint32_t serial) override;
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
  PlatformImeController* GetPlatformImeController() override;

  // PlatformEventDispatcher:
  bool CanDispatchEvent(const PlatformEvent& event) override;
  uint32_t DispatchEvent(const PlatformEvent& event) override;

  // ChannelObserver:
  void OnChannelEstablished() override;
  void OnChannelDestroyed() override;

 private:
  void SendWidgetState();
  void AddRegion();
  void ResetRegion();
  void SetCursor();
  void ValidateBounds();
  PlatformWindowDelegate* delegate_;   // Not owned.
  OzoneGpuPlatformSupportHost* sender_;  // Not owned.
  WindowManagerWayland* window_manager_;  // Not owned.
  bool transparent_;
  gfx::Rect bounds_;
  unsigned handle_;
  unsigned parent_;
  ui::WidgetType type_;
  ui::WidgetState state_;
  SkRegion* region_;
  int cursor_type_;
  base::string16 title_;
  // The current cursor bitmap (immutable).
  scoped_refptr<BitmapCursorOzone> bitmap_;

  DISALLOW_COPY_AND_ASSIGN(OzoneWaylandWindow);
};

}  // namespace ui

#endif  // OZONE_PLATFORM_OZONE_WAYLAND_WINDOW_H_
