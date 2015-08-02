// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_EVENTS_WINDOW_CONSTANTS_H_
#define OZONE_UI_EVENTS_WINDOW_CONSTANTS_H_

namespace ui {

  enum WidgetState {
    UNINITIALIZED = 0,
    SHOW = 1,  // Widget is visible.
    HIDE = 2,  // Widget is hidden.
    FULLSCREEN = 3,  // Widget is in fullscreen mode.
    MAXIMIZED = 4,  // Widget is maximized,
    MINIMIZED = 5,  // Widget is minimized.
    RESTORE = 6,  // Restore Widget.
    ACTIVE = 7,  // Widget is Activated.
    INACTIVE = 8,  // Widget is DeActivated.
    RESIZE = 9,  // Widget is Resized.
    DESTROYED = 10  // Widget is Destroyed.
  };

  enum WidgetType {
    WINDOW = 1,  // A decorated Window.
    WINDOWFRAMELESS = 2,  // An undecorated Window.
    POPUP = 3,  // An undecorated Window, with transient positioning relative to
      // its parent and in which the input pointer is implicit grabbed
      // (i.e. Wayland install the grab) by the Window.
    TOOLTIP = 4
  };

}  // namespace ui

#endif  // OZONE_UI_EVENTS_WINDOW_CONSTANTS_H_
