// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/desktop_aura/desktop_screen_wayland.h"

#include "ui/ozone/public/ozone_platform.h"
#include "ui/platform_window/platform_window.h"
#include "ozone/ui/desktop_aura/desktop_window_tree_host_ozone.h"
#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ui/aura/window.h"

namespace views {

DesktopScreenWayland::DesktopScreenWayland()
    : gfx::Screen(),
      rect_(0, 0, 0, 0),
      displays_() {
  ui::EventFactoryOzoneWayland::GetInstance()->SetOutputChangeObserver(this);
  ui::OzonePlatform::GetInstance()->CreatePlatformWindow(NULL, gfx::Rect());
}

DesktopScreenWayland::~DesktopScreenWayland() {
}

void DesktopScreenWayland::SetGeometry(const gfx::Rect& geometry) {
  rect_ = geometry;
  int max_area = 0;
  const gfx::Display* matching = NULL;
  for (std::vector<gfx::Display>::const_iterator it = displays_.begin();
       it != displays_.end(); ++it) {
    gfx::Rect intersect = gfx::IntersectRects(it->bounds(), rect_);
    int area = intersect.width() * intersect.height();
    if (area > max_area) {
      max_area = area;
      matching = &*it;
    }
  }

  if (!matching) {
    displays_.push_back(gfx::Display(displays_.size(), rect_));
    change_notifier_.NotifyDisplaysChanged(displays_, displays_);
  }
}

gfx::Point DesktopScreenWayland::GetCursorScreenPoint() {
  return gfx::Point();
}

gfx::NativeWindow DesktopScreenWayland::GetWindowUnderCursor() {
  return NULL;
}

gfx::NativeWindow DesktopScreenWayland::GetWindowAtScreenPoint(
    const gfx::Point& point) {
  const std::vector<aura::Window*>& windows =
      views::DesktopWindowTreeHostOzone::GetAllOpenWindows();
  for (std::vector<aura::Window*>::const_iterator it = windows.begin();
         it != windows.end(); ++it) {
    if ((*it)->GetBoundsInScreen().Contains(point))
      return *it;
  }

  return NULL;
}

int DesktopScreenWayland::GetNumDisplays() const {
  return displays_.size();
}

std::vector<gfx::Display> DesktopScreenWayland::GetAllDisplays() const {
  return displays_;
}

gfx::Display DesktopScreenWayland::GetDisplayNearestWindow(
    gfx::NativeView window) const {
  DCHECK(!rect_.IsEmpty());
  if (displays_.size() == 1)
    return displays_.front();
  // Getting screen bounds here safely is hard.
  //
  // You'd think we'd be able to just call window->GetBoundsInScreen(), but we
  // can't because |window| (and the associated RootWindow*) can be partially
  // initialized at this point; RootWindow initializations call through into
  // GetDisplayNearestWindow(). But the wayland resources are created before we
  // create the aura::RootWindow. So we ask what the DRWH believes the
  // window bounds are instead of going through the aura::Window's screen
  // bounds.
  aura::WindowTreeHost* host = window->GetHost();
  if (host) {
    DesktopWindowTreeHostOzone* rwh =
        DesktopWindowTreeHostOzone::GetHostForAcceleratedWidget(
            host->GetAcceleratedWidget());
    if (rwh)
      return GetDisplayMatching(rwh->GetBoundsInScreen());
  }

  return GetPrimaryDisplay();
}

gfx::Display DesktopScreenWayland::GetDisplayNearestPoint(
    const gfx::Point& point) const {
  if (displays_.size() == 1)
    return displays_.front();

  for (std::vector<gfx::Display>::const_iterator it = displays_.begin();
         it != displays_.end(); ++it) {
    if (it->bounds().Contains(point))
      return *it;
  }

  return GetPrimaryDisplay();
}

gfx::Display DesktopScreenWayland::GetDisplayMatching(
    const gfx::Rect& match_rect) const {
  if (displays_.size() == 1)
    return displays_.front();

  DCHECK(!rect_.IsEmpty());
  int max_area = 0;
  const gfx::Display* matching = NULL;
  for (std::vector<gfx::Display>::const_iterator it = displays_.begin();
       it != displays_.end(); ++it) {
    gfx::Rect intersect = gfx::IntersectRects(it->bounds(), match_rect);
    int area = intersect.width() * intersect.height();
    if (area > max_area) {
      max_area = area;
      matching = &*it;
    }
  }

  // Fallback to the primary display if there is no matching display.
  return matching ? *matching : GetPrimaryDisplay();
}

gfx::Display DesktopScreenWayland::GetPrimaryDisplay() const {
  DCHECK(!rect_.IsEmpty());
  return displays_.front();
}

void DesktopScreenWayland::AddObserver(gfx::DisplayObserver* observer) {
  change_notifier_.AddObserver(observer);
}

void DesktopScreenWayland::RemoveObserver(gfx::DisplayObserver* observer) {
  change_notifier_.RemoveObserver(observer);
}

void DesktopScreenWayland::OnOutputSizeChanged(unsigned width,
                                               unsigned height) {
  SetGeometry(gfx::Rect(0, 0, width, height));
}

gfx::Screen* CreateDesktopScreen() {
  return new DesktopScreenWayland();
}

}  // namespace views
