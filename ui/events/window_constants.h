// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_EVENTS_WINDOW_CONSTANTS_H_
#define OZONE_UI_EVENTS_WINDOW_CONSTANTS_H_

namespace ozonewayland {

  enum WidgetState {
    CREATE = 1,  // Create a new Widget
    SHOW = 2,  // Widget is visible.
    HIDE = 3,  // Widget is hidden.
    FULLSCREEN = 4,  // Widget is in fullscreen mode.
    MAXIMIZED = 5,  // Widget is maximized,
    MINIMIZED = 6,  // Widget is minimized.
    RESTORE = 7,  // Restore Widget.
    ACTIVE = 8,  // Widget is Activated.
    INACTIVE = 9,  // Widget is DeActivated.
    RESIZE = 10,  // Widget is Resized.
    DESTROYED = 11  // Widget is Destroyed.
  };

  enum WidgetType {
    WINDOW = 1,  // A decorated Window.
    WINDOWFRAMELESS = 2,  // An undecorated Window.
    POPUP = 3  // An undecorated Window, with transient positioning relative to
      // its parent and in which the input pointer is implicit grabbed
      // (i.e. Wayland install the grab) by the Window.
  };

}  // namespace ozonewayland

#endif  // OZONE_UI_EVENTS_WINDOW_CONSTANTS_H_
