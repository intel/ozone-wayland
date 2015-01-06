// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_DESKTOP_AURA_DESKTOP_SCREEN_WAYLAND_H__
#define OZONE_IMPL_DESKTOP_AURA_DESKTOP_SCREEN_WAYLAND_H__

#include <vector>

#include "ozone/ui/events/output_change_observer.h"
#include "ui/gfx/display_change_notifier.h"
#include "ui/gfx/screen.h"

namespace aura {
class Window;
}

namespace ui {
class DesktopPlatformScreen;
}

namespace views {

class DesktopScreenWayland : public gfx::Screen,
                             public ui::OutputChangeObserver {
 public:
  DesktopScreenWayland();
  ~DesktopScreenWayland() override;

  // OutputChangeObserver overrides.
  void OnOutputSizeChanged(unsigned width, unsigned height) override;

 private:
  void SetGeometry(const gfx::Rect& geometry);
  // Overridden from gfx::Screen:
  gfx::Point GetCursorScreenPoint() override;
  gfx::NativeWindow GetWindowUnderCursor() override;
  gfx::NativeWindow GetWindowAtScreenPoint(const gfx::Point& point) override;
  int GetNumDisplays() const override;
  std::vector<gfx::Display> GetAllDisplays() const override;
  gfx::Display GetDisplayNearestWindow(gfx::NativeView window) const override;
  gfx::Display GetDisplayNearestPoint(const gfx::Point& point) const override;
  gfx::Display GetDisplayMatching(const gfx::Rect& match_rect) const override;
  gfx::Display GetPrimaryDisplay() const override;
  void AddObserver(gfx::DisplayObserver* observer) override;
  void RemoveObserver(gfx::DisplayObserver* observer) override;

  gfx::Rect rect_;
  gfx::DisplayChangeNotifier change_notifier_;

  // The display objects we present to chrome.
  std::vector<gfx::Display> displays_;
  ui::DesktopPlatformScreen* platform_Screen_;
  DISALLOW_COPY_AND_ASSIGN(DesktopScreenWayland);
};

}  // namespace views

#endif  // OZONE_IMPL_DESKTOP_AURA_DESKTOP_SCREEN_WAYLAND_H__
