// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_DESKTOP_AURA_DESKTOP_SCREEN_WAYLAND_H__
#define OZONE_IMPL_DESKTOP_AURA_DESKTOP_SCREEN_WAYLAND_H__

#include <vector>

#include "ozone/platform/desktop_platform_screen_delegate.h"
#include "ui/display/display_change_notifier.h"
#include "ui/display/screen.h"
#include "ui/gfx/geometry/rect.h"

namespace aura {
class Window;
}

namespace ui {
class DesktopPlatformScreen;
}

namespace views {

class DesktopScreenWayland : public display::Screen,
                             public ui::DesktopPlatformScreenDelegate {
 public:
  DesktopScreenWayland();
  ~DesktopScreenWayland() override;

  // DesktopPlatformScreenDelegate overrides.
  void OnOutputSizeChanged(unsigned width, unsigned height) override;

 private:
  void SetGeometry(const gfx::Rect& geometry);
  // Overridden from gfx::Screen:
  gfx::Point GetCursorScreenPoint() override;
  bool IsWindowUnderCursor(gfx::NativeWindow window) override;
  gfx::NativeWindow GetWindowUnderCursor();
  gfx::NativeWindow GetWindowAtScreenPoint(const gfx::Point& point) override;
  int GetNumDisplays() const override;
  std::vector<display::Display> GetAllDisplays() const override;
  display::Display GetDisplayNearestWindow(gfx::NativeView window) const override;
  display::Display GetDisplayNearestPoint(const gfx::Point& point) const override;
  display::Display GetDisplayMatching(const gfx::Rect& match_rect) const override;
  display::Display GetPrimaryDisplay() const override;
  void AddObserver(display::DisplayObserver* observer) override;
  void RemoveObserver(display::DisplayObserver* observer) override;

  gfx::Rect rect_;
  display::DisplayChangeNotifier change_notifier_;

  // The display objects we present to chrome.
  std::vector<display::Display> displays_;
  std::unique_ptr<ui::DesktopPlatformScreen> platform_Screen_;
  DISALLOW_COPY_AND_ASSIGN(DesktopScreenWayland);
};

}  // namespace views

#endif  // OZONE_IMPL_DESKTOP_AURA_DESKTOP_SCREEN_WAYLAND_H__
