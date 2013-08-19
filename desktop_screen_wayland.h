// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DESKTOP_SCREEN_WAYLAND_H__
#define DESKTOP_SCREEN_WAYLAND_H__

#include "ui/gfx/screen.h"

namespace ui {
class DesktopScreenWayland : public gfx::Screen {
 public:
  DesktopScreenWayland();
  virtual ~DesktopScreenWayland();

 private:
  // Overridden from gfx::Screen:
  virtual bool IsDIPEnabled() OVERRIDE;
  virtual gfx::Point GetCursorScreenPoint() OVERRIDE;
  virtual gfx::NativeWindow GetWindowAtCursorScreenPoint() OVERRIDE;
  virtual int GetNumDisplays() OVERRIDE;
  virtual gfx::Display GetDisplayNearestWindow(
      gfx::NativeView window) const OVERRIDE;
  virtual gfx::Display GetDisplayNearestPoint(
      const gfx::Point& point) const OVERRIDE;
  virtual gfx::Display GetDisplayMatching(
      const gfx::Rect& match_rect) const OVERRIDE;
  virtual gfx::Display GetPrimaryDisplay() const OVERRIDE;
  virtual void AddObserver(gfx::DisplayObserver* observer) OVERRIDE;
  virtual void RemoveObserver(gfx::DisplayObserver* observer) OVERRIDE;

  DISALLOW_COPY_AND_ASSIGN(DesktopScreenWayland);
};
}

#endif  // DESKTOP_SCREEN_WAYLAND_H__
