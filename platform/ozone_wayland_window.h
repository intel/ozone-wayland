// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_PLATFORM_OZONE_WAYLAND_WINDOW_H_
#define OZONE_PLATFORM_OZONE_WAYLAND_WINDOW_H_

#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/platform_window/platform_window.h"

namespace views {
class WindowTreeHostDelegateWayland;
}

namespace ui {

class PlatformWindowDelegate;

class OzoneWaylandWindow : public PlatformWindow {
 public:
  OzoneWaylandWindow(PlatformWindowDelegate* delegate,
                     const gfx::Rect& bounds);
  virtual ~OzoneWaylandWindow();

  unsigned GetHandle() const { return handle_; }
  PlatformWindowDelegate* GetDelegate() const { return delegate_; }

  // PlatformWindow:
  virtual void InitPlatformWindow(
       PlatformWindowType type, gfx::AcceleratedWidget parent_window) override;
  virtual gfx::Rect GetBounds() override;
  virtual void SetBounds(const gfx::Rect& bounds) override;
  virtual void Show() override;
  virtual void Hide() override;
  virtual void Close() override;
  virtual void SetCapture() override;
  virtual void ReleaseCapture() override;
  virtual void ToggleFullscreen() override;
  virtual void Maximize() override;
  virtual void Minimize() override;
  virtual void Restore() override;
  virtual void SetCursor(PlatformCursor cursor) override;
  virtual void MoveCursorTo(const gfx::Point& location) override;

 private:
  PlatformWindowDelegate* delegate_;
  gfx::Rect bounds_;
  unsigned handle_;

  static views::WindowTreeHostDelegateWayland* g_delegate_ozone_wayland_;
  DISALLOW_COPY_AND_ASSIGN(OzoneWaylandWindow);
};

}  // namespace ui

#endif  // OZONE_PLATFORM_OZONE_WAYLAND_WINDOW_H_
