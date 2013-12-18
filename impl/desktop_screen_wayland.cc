// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/desktop_screen_wayland.h"

namespace ozonewayland {

DesktopScreenWayland::DesktopScreenWayland() {
}

DesktopScreenWayland::~DesktopScreenWayland() {
}

void DesktopScreenWayland::SetGeometry(const gfx::Rect& geometry) {
  rect_ = geometry;
}

bool DesktopScreenWayland::IsDIPEnabled() {
  return false;
}

gfx::Point DesktopScreenWayland::GetCursorScreenPoint() {
  return gfx::Point();
}

gfx::NativeWindow DesktopScreenWayland::GetWindowUnderCursor() {
  return NULL;
}

gfx::NativeWindow DesktopScreenWayland::GetWindowAtScreenPoint(
    const gfx::Point& point) {
  return NULL;
}

int DesktopScreenWayland::GetNumDisplays() const {
  return 1;
}

std::vector<gfx::Display> DesktopScreenWayland::GetAllDisplays() const {
  return std::vector<gfx::Display>(1, GetPrimaryDisplay());
}

gfx::Display DesktopScreenWayland::GetDisplayNearestWindow(
    gfx::NativeView window) const {
  DCHECK(!rect_.IsEmpty());
  return gfx::Display(0, rect_);
}

gfx::Display DesktopScreenWayland::GetDisplayNearestPoint(
    const gfx::Point& point) const {
  DCHECK(!rect_.IsEmpty());
  return gfx::Display(0, rect_);
}

gfx::Display DesktopScreenWayland::GetDisplayMatching(
    const gfx::Rect& match_rect) const {
  DCHECK(!rect_.IsEmpty());
  return gfx::Display(0, rect_);
}

gfx::Display DesktopScreenWayland::GetPrimaryDisplay() const {
  DCHECK(!rect_.IsEmpty());
  return gfx::Display(0, rect_);
}

void DesktopScreenWayland::AddObserver(gfx::DisplayObserver* observer) {
}

void DesktopScreenWayland::RemoveObserver(gfx::DisplayObserver* observer) {
}

}  // namespace ozonewayland
