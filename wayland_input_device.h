// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_INPUT_DEVICE_H_
#define OZONE_WAYLAND_INPUT_DEVICE_H_

#include <stdint.h>

#include <xkbcommon/xkbcommon.h>
#include "base/basictypes.h"
#include "ui/gfx/point.h"
#include "ozone/wayland_delegate.h"

#define MOD_SHIFT_MASK		0x01
#define MOD_ALT_MASK		0x02
#define MOD_CONTROL_MASK	0x04

struct wl_array;
struct wl_buffer;
struct wl_display;
struct wl_input_device;
struct wl_surface;

namespace ui {

class WaylandWindow;

enum CursorType {
  CURSOR_BOTTOM_LEFT,
  CURSOR_BOTTOM_RIGHT,
  CURSOR_BOTTOM,
  CURSOR_DRAGGING,
  CURSOR_LEFT_PTR,
  CURSOR_LEFT,
  CURSOR_RIGHT,
  CURSOR_TOP_LEFT,
  CURSOR_TOP_RIGHT,
  CURSOR_TOP,
  CURSOR_IBEAM,
  CURSOR_HAND1,
};

// Constants to identify the type of resize.
enum BoundsChangeType
{
  kBoundsChange_None = 0,
  kBoundsChange_Repositions,
  kBoundsChange_Resizes,
};

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

// This class represents an input device that was registered with Wayland.
// The purpose of this class is to parse and wrap events into generic
// WaylandEvent types and dispatch the event to the appropriate WaylandWindow.
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
  static int GetPointerImageForWindowComponent(int component);

  void SetCurrentPointerImage(int pointer) { current_pointer_image_ = pointer; }
  int GetCurrentPointerImage() { return current_pointer_image_; }

  wl_seat* GetInputSeat() { return input_seat_; }
  wl_pointer* GetPointer() { return input_pointer_; }

 private:
  // Input device callback functions. These will create 'WaylandEvent's and
  // send them to the currently focused window.
  // Args:
  //  - data: Pointer to the WaylandInputDevice object associated with the
  //          'input_device'
  //  - input_device:
  //          The input device that sent the event
  //  - time: The time of the event.
  static void OnMotionNotify(void* data,
      wl_pointer* input_pointer,
      uint32_t time,
      wl_fixed_t sx_w,
      wl_fixed_t sy_w);

  static void OnButtonNotify(void* data,
      wl_pointer* input_pointer,
      uint32_t serial,
      uint32_t time,
      uint32_t button,
      uint32_t state);

  static void OnKeyNotify(void* data,
      wl_keyboard* input_keyboard,
      uint32_t serial,
      uint32_t time,
      uint32_t key,
      uint32_t state);

  static void OnAxisNotify(void* data,
      wl_pointer* input_pointer,
      uint32_t time,
      uint32_t axis,
      int32_t value);

  // On*Focus events also have a Wayland surface associated with them. If the
  // surface is NULL, then the event signifies a loss of focus. Otherwise we
  // use the surface to get the WaylandWindow that receives focus.
  static void OnPointerEnter(void* data,
      wl_pointer* input_pointer,
      uint32_t serial,
      wl_surface* surface,
      wl_fixed_t sx_w,
      wl_fixed_t sy_w);
  static void OnPointerLeave(void* data,
      wl_pointer* input_pointer,
      uint32_t serial,
      wl_surface* surface);
  static void OnKeyboardKeymap(void *data,
      struct wl_keyboard *keyboard,
      uint32_t format, int fd, uint32_t size);
  static void OnKeyboardEnter(void* data,
      wl_keyboard* input_keyboard,
      uint32_t serial,
      wl_surface* surface,
      wl_array* keys);
  static void OnKeyboardLeave(void* data,
      wl_keyboard* input_keyboard,
      uint32_t serial,
      wl_surface* surface);
  static void OnKeyModifiers(void *data, wl_keyboard *keyboard,
      uint32_t serial, uint32_t mods_depressed,
      uint32_t mods_latched, uint32_t mods_locked,
      uint32_t group);

  static void OnSeatCapabilities(void *data,
      wl_seat *seat,
      uint32_t caps);

  wl_seat* input_seat_;
  wl_pointer* input_pointer_;
  wl_keyboard* input_keyboard_;
  wl_display* display_;
  wl_registry* registry_;

  // These keep track of the window that's currently under focus. NULL if no
  // window is under focus.
  WaylandWindow* pointer_focus_;
  WaylandWindow* keyboard_focus_;

  // Keeps track of the currently active keyboard modifiers. We keep this
  // since we want to advertise keyboard modifiers with mouse events.
  uint32_t keyboard_modifiers_;

  // Keeps track of the last position for the motion event. We want to
  // publish this with events such as button notify which doesn't have a
  // position associated by default.
  gfx::Point global_position_;
  gfx::Point surface_position_;

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

  int current_pointer_image_;

  DISALLOW_COPY_AND_ASSIGN(WaylandInputDevice);
};

}  // namespace ui

#endif  // OZONE_WAYLAND_INPUT_DEVICE_H_
