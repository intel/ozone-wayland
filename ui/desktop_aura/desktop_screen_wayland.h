// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_DESKTOP_AURA_DESKTOP_SCREEN_WAYLAND_H__
#define OZONE_IMPL_DESKTOP_AURA_DESKTOP_SCREEN_WAYLAND_H__

#include <vector>

#include "ozone/ui/events/output_change_observer.h"
#include "ui/gfx/screen.h"

namespace aura {
class Window;
}

namespace views {

class DesktopScreenWayland : public gfx::Screen,
                             public ui::OutputChangeObserver {
 public:
  DesktopScreenWayland();
  virtual ~DesktopScreenWayland();

  // OutputChangeObserver overrides.
  virtual void OnOutputSizeChanged(unsigned width, unsigned height) override;

 private:
  void SetGeometry(const gfx::Rect& geometry);
  // Overridden from gfx::Screen:
  virtual gfx::Point GetCursorScreenPoint() override;
  virtual gfx::NativeWindow GetWindowUnderCursor() override;
  virtual gfx::NativeWindow GetWindowAtScreenPoint(const gfx::Point& point)
      override;
  virtual int GetNumDisplays() const override;
  virtual std::vector<gfx::Display> GetAllDisplays() const override;
  virtual gfx::Display GetDisplayNearestWindow(
      gfx::NativeView window) const override;
  virtual gfx::Display GetDisplayNearestPoint(
      const gfx::Point& point) const override;
  virtual gfx::Display GetDisplayMatching(
      const gfx::Rect& match_rect) const override;
  virtual gfx::Display GetPrimaryDisplay() const override;
  virtual void AddObserver(gfx::DisplayObserver* observer) override;
  virtual void RemoveObserver(gfx::DisplayObserver* observer) override;

  gfx::Rect rect_;
  // The display objects we present to chrome.
  std::vector<gfx::Display> displays_;
  DISALLOW_COPY_AND_ASSIGN(DesktopScreenWayland);
};

}  // namespace views

#endif  // OZONE_IMPL_DESKTOP_AURA_DESKTOP_SCREEN_WAYLAND_H__
