// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/desktop_screen_wayland.h"

#include "ui/base/ozone/surface_factory_ozone.h"
#include <stdio.h>

namespace {

gfx::Size GetPrimaryDisplaySize() {
  int width, height;
  const char* display =
      ui::SurfaceFactoryOzone::GetInstance()->DefaultDisplaySpec();

  int sc = sscanf(display, "%dx%d", &width, &height);
  if (sc != 2) {
    LOG(WARNING) << "malformed display spec from "
                 << "SurfaceFactoryOzone::DefaultDisplaySpec";
    return gfx::Size();
  }

  return gfx::Size(width, height);
}
}  // namespace

DesktopScreenWayland::DesktopScreenWayland() {
}

DesktopScreenWayland::~DesktopScreenWayland() {
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
  return gfx::Display(0, gfx::Rect(GetPrimaryDisplaySize()));
}

gfx::Display DesktopScreenWayland::GetDisplayNearestPoint(
    const gfx::Point& point) const {
  return gfx::Display(0, gfx::Rect(GetPrimaryDisplaySize()));
}

gfx::Display DesktopScreenWayland::GetDisplayMatching(
    const gfx::Rect& match_rect) const {
  return gfx::Display(0, gfx::Rect(GetPrimaryDisplaySize()));
}

gfx::Display DesktopScreenWayland::GetPrimaryDisplay() const {
  return gfx::Display(0, gfx::Rect(GetPrimaryDisplaySize()));
}

void DesktopScreenWayland::AddObserver(gfx::DisplayObserver* observer) {
}

void DesktopScreenWayland::RemoveObserver(gfx::DisplayObserver* observer) {
}
