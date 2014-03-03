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

namespace ozonewayland {

class DesktopScreenWayland : public gfx::Screen,
                             public ui::OutputChangeObserver {
 public:
  DesktopScreenWayland();
  virtual ~DesktopScreenWayland();

  // OutputChangeObserver overrides.
  virtual void OnOutputSizeChanged(unsigned width, unsigned height) OVERRIDE;

 private:
  void SetGeometry(const gfx::Rect& geometry);
  // Overridden from gfx::Screen:
  virtual bool IsDIPEnabled() OVERRIDE;
  virtual gfx::Point GetCursorScreenPoint() OVERRIDE;
  virtual gfx::NativeWindow GetWindowUnderCursor() OVERRIDE;
  virtual gfx::NativeWindow GetWindowAtScreenPoint(const gfx::Point& point)
      OVERRIDE;
  virtual int GetNumDisplays() const OVERRIDE;
  virtual std::vector<gfx::Display> GetAllDisplays() const OVERRIDE;
  virtual gfx::Display GetDisplayNearestWindow(
      gfx::NativeView window) const OVERRIDE;
  virtual gfx::Display GetDisplayNearestPoint(
      const gfx::Point& point) const OVERRIDE;
  virtual gfx::Display GetDisplayMatching(
      const gfx::Rect& match_rect) const OVERRIDE;
  virtual gfx::Display GetPrimaryDisplay() const OVERRIDE;
  virtual void AddObserver(gfx::DisplayObserver* observer) OVERRIDE;
  virtual void RemoveObserver(gfx::DisplayObserver* observer) OVERRIDE;

  gfx::Rect rect_;
  // The display objects we present to chrome.
  std::vector<gfx::Display> displays_;
  DISALLOW_COPY_AND_ASSIGN(DesktopScreenWayland);
};

}  // namespace ozonewayland

#endif  // OZONE_IMPL_DESKTOP_AURA_DESKTOP_SCREEN_WAYLAND_H__
