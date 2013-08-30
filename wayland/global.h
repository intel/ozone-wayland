// Copyright (C) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_GLOBAL_H_
#define OZONE_WAYLAND_GLOBAL_H_

namespace ui {

#define MOD_SHIFT_MASK		0x01
#define MOD_ALT_MASK		0x02
#define MOD_CONTROL_MASK	0x04

enum WindowLocation {
  WINDOW_INTERIOR = 0,
  WINDOW_RESIZING_TOP = 1,
  WINDOW_RESIZING_BOTTOM = 2,
  WINDOW_RESIZING_LEFT = 4,
  WINDOW_RESIZING_TOP_LEFT = 5,
  WINDOW_RESIZING_BOTTOM_LEFT = 6,
  WINDOW_RESIZING_RIGHT = 8,
  WINDOW_RESIZING_TOP_RIGHT = 9,
  WINDOW_RESIZING_BOTTOM_RIGHT = 10,
  WINDOW_RESIZING_MASK = 15,
  WINDOW_EXTERIOR = 16,
  WINDOW_TITLEBAR = 17,
  WINDOW_CLIENT_AREA = 18,
 };

// Constants to identify the type of resize.
enum BoundsChangeType
{
  kBoundsChange_None = 0,
  kBoundsChange_Repositions,
  kBoundsChange_Resizes,
};

}  // namespace ui

#endif  // OZONE_WAYLAND_DISPLAY_H_

