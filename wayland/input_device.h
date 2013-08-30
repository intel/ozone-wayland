// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Copyright (C) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_INPUT_DEVICE_H_
#define OZONE_WAYLAND_INPUT_DEVICE_H_

#include <stdint.h>

#include <xkbcommon/xkbcommon.h>
#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "ozone/wayland/display.h"
#include "ozone/wayland/global.h"
#include "ui/gfx/point.h"

namespace ui {

class Event;
class WaylandWindow;
class WaylandCursor;

// This class represents an input device that was registered with Wayland.
// The purpose of this class is to parse and wrap events into generic
// ui.Event types and dispatch the event to the appropriate window that has
// assigned to the dispatcher.
//
// How Wayland events work:
// ------------------------
//
// When the On*Focus events are triggered, the input device receives a
// reference to the surface that just received/lost focus. Each surface is
// associated with a unique WaylandWindow. When processing the focus events we
// keep track of the currently focused window such that when we receive
// different events (mouse button press or key press) we only send the event to
// the window in focus.
class WaylandInputDevice {
 public:
  WaylandInputDevice(WaylandDisplay* display, uint32_t id);
  ~WaylandInputDevice();

  // Returns a bitmask of the kBoundsChange_ values.
  static BoundsChangeType GetBoundsChangeForWindowComponent(int component);
  static WindowLocation GetLocationForWindowComponent(int component);

  wl_seat* GetInputSeat() { return input_seat_; }
  WaylandCursor* cursor() { return cursor_; }

 private:
  static void DispatchEventHelper(scoped_ptr<ui::Event> key);
  static void DispatchEvent(scoped_ptr<ui::Event> event);

  static void OnMotionNotify(
      void* data,
      wl_pointer* input_pointer,
      uint32_t time,
      wl_fixed_t sx_w,
      wl_fixed_t sy_w);

  static void OnButtonNotify(
      void* data,
      wl_pointer* input_pointer,
      uint32_t serial,
      uint32_t time,
      uint32_t button,
      uint32_t state);

  static void OnKeyNotify(
      void* data,
      wl_keyboard* input_keyboard,
      uint32_t serial,
      uint32_t time,
      uint32_t key,
      uint32_t state);

  static void OnAxisNotify(
      void* data,
      wl_pointer* input_pointer,
      uint32_t time,
      uint32_t axis,
      int32_t value);

  // On*Focus events also have a Wayland surface associated with them. If the
  // surface is NULL, then the event signifies a loss of focus. Otherwise we
  // use the surface to get the WaylandWindow that receives focus.
  static void OnPointerEnter(
      void* data,
      wl_pointer* input_pointer,
      uint32_t serial,
      wl_surface* surface,
      wl_fixed_t sx_w,
      wl_fixed_t sy_w);

  static void OnPointerLeave(
      void* data,
      wl_pointer* input_pointer,
      uint32_t serial,
      wl_surface* surface);

  static void OnKeyboardKeymap(
      void *data,
      struct wl_keyboard *keyboard,
      uint32_t format,
      int fd,
      uint32_t size);

  static void OnKeyboardEnter(
      void* data,
      wl_keyboard* input_keyboard,
      uint32_t serial,
      wl_surface* surface,
      wl_array* keys);

  static void OnKeyboardLeave(
      void* data,
      wl_keyboard* input_keyboard,
      uint32_t serial,
      wl_surface* surface);

  static void OnKeyModifiers(
      void *data,
      wl_keyboard *keyboard,
      uint32_t serial,
      uint32_t mods_depressed,
      uint32_t mods_latched,
      uint32_t mods_locked,
      uint32_t group);

  static void OnSeatCapabilities(
      void *data,
      wl_seat *seat,
      uint32_t caps);

  wl_seat* input_seat_;
  wl_keyboard* input_keyboard_;

  WaylandCursor* cursor_;

  // These keep track of the window that's currently under focus. NULL if no
  // window is under focus.
  WaylandWindow* pointer_focus_;
  WaylandWindow* keyboard_focus_;

  // Keeps track of the currently active keyboard modifiers. We keep this
  // since we want to advertise keyboard modifiers with mouse events.
  uint32_t keyboard_modifiers_;

  // Keeps track of the last position for the motion event. We want to
  // dispatch this with events such as wheel or button which don't have a
  // position associated on Wayland.
  gfx::Point pointer_position_;

  // Keep track of the time of last event. Useful when we get buffer Attach
  // calls and the calls wouldn't have a way of getting an event time.
  uint32_t pointer_enter_serial_;

  // keymap used to transform keyboard events.
  struct xkb {
    struct xkb_rule_names names;
    struct xkb_keymap *keymap;
    struct xkb_state *state;
    struct xkb_context *context;
    xkb_mod_mask_t control_mask;
    xkb_mod_mask_t alt_mask;
    xkb_mod_mask_t shift_mask;
  };

  xkb xkb_;

  void InitXKB();
  void FiniXKB();

  DISALLOW_COPY_AND_ASSIGN(WaylandInputDevice);
};

}  // namespace ui

#endif  // OZONE_WAYLAND_INPUT_DEVICE_H_
