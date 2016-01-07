// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/desktop_aura/ozone_util.h"

#include <set>
#include <vector>

#include "chrome/browser/ui/views/tabs/window_finder.h"
#include "ozone/ui/desktop_aura/desktop_window_tree_host_ozone.h"

namespace ui {

VIEWS_EXPORT bool GetCustomFramePrefDefault() {
  return true;
}

}  // namespace ui

VIEWS_EXPORT bool IsFullScreenMode() {
  std::vector<aura::Window*> all_windows =
      views::DesktopWindowTreeHostOzone::GetAllOpenWindows();
  // Only the topmost window is checked. This works fine in the most cases, but
  // it may return false when there are multiple displays and one display has
  // a fullscreen window but others don't. See: crbug.com/345484
  if (all_windows.empty())
    return false;

  views::Widget* widget =
      views::Widget::GetWidgetForNativeWindow(all_windows[0]);
  return widget && widget->IsFullscreen();
}

// TODO(Kalyan): Remove this once https://codereview.chromium.org/176893002/
// is fixed.
VIEWS_EXPORT gfx::NativeWindow GetLocalProcessWindowAtPoint(
    chrome::HostDesktopType host_desktop_type,
    const gfx::Point& screen_point,
    const std::set<gfx::NativeWindow>& ignore,
    gfx::NativeWindow source) {
  return NULL;
}
