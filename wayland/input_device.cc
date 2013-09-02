// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/input_device.h"

#include <sys/mman.h>
#include <linux/input.h>
#include <wayland-client.h>

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_pump_ozone.h"
#include "ozone/wayland/cursor.h"
#include "ozone/wayland/kbd_conversion.h"
#include "ozone/wayland/window.h"
#include "ui/base/events/event.h"
#include "ui/base/hit_test.h"

namespace ui {

// static
BoundsChangeType WaylandInputDevice::GetBoundsChangeForWindowComponent(int component)
{
  BoundsChangeType bounds_change = kBoundsChange_None;
  switch (component) {
    case HTCAPTION:
      bounds_change = kBoundsChange_Repositions;
      break;
    case HTTOPLEFT:
    case HTTOP:
    case HTTOPRIGHT:
    case HTLEFT:
    case HTBOTTOMLEFT:
    case HTRIGHT:
    case HTBOTTOMRIGHT:
    case HTBOTTOM:
      //case HTGROWBOX:
      bounds_change = kBoundsChange_Resizes;
      break;
    default:
      break;
  }
  return bounds_change;
}

// static
WindowLocation WaylandInputDevice::GetLocationForWindowComponent(int component)
{
  WindowLocation location = WINDOW_INTERIOR;
  switch (component) {
    case HTCAPTION:
      location = WINDOW_TITLEBAR;
      break;
    case HTTOPLEFT:
      location = WINDOW_RESIZING_TOP_LEFT;
      break;
    case HTTOP:
      location = WINDOW_RESIZING_TOP;
      break;
    case HTTOPRIGHT:
      location = WINDOW_RESIZING_TOP_RIGHT;
      break;
    case HTLEFT:
      location = WINDOW_RESIZING_LEFT;
      break;
    case HTBOTTOMLEFT:
      location = WINDOW_RESIZING_BOTTOM_LEFT;
      break;
    case HTRIGHT:
      location = WINDOW_RESIZING_RIGHT;
      break;
    case HTBOTTOMRIGHT:
      location = WINDOW_RESIZING_BOTTOM_RIGHT;
      break;
    case HTBOTTOM:
      location = WINDOW_RESIZING_BOTTOM;
      break;
    default:
      break;
  }
  return location;
}

static WaylandCursor::CursorType cursorType(WindowLocation location)
{
    switch (location) {
      case WINDOW_RESIZING_TOP:
        return WaylandCursor::CURSOR_TOP;
        break;
      case WINDOW_RESIZING_BOTTOM:
        return WaylandCursor::CURSOR_BOTTOM;
        break;
      case WINDOW_RESIZING_LEFT:
        return WaylandCursor::CURSOR_LEFT;
        break;
      case WINDOW_RESIZING_RIGHT:
        return WaylandCursor::CURSOR_RIGHT;
        break;
      case WINDOW_RESIZING_TOP_LEFT:
        return WaylandCursor::CURSOR_TOP_LEFT;
        break;
      case WINDOW_RESIZING_TOP_RIGHT:
        return WaylandCursor::CURSOR_TOP_RIGHT;
        break;
      case WINDOW_RESIZING_BOTTOM_LEFT:
        return WaylandCursor::CURSOR_BOTTOM_LEFT;
        break;
      case WINDOW_RESIZING_BOTTOM_RIGHT:
        return WaylandCursor::CURSOR_BOTTOM_RIGHT;
        break;
      case WINDOW_EXTERIOR:
      case WINDOW_TITLEBAR:
        default:
        return WaylandCursor::CURSOR_LEFT_PTR;
      }
}

WaylandInputDevice::WaylandInputDevice(WaylandDisplay* display, uint32_t id)
  : input_seat_(NULL),
    input_keyboard_(NULL),
    pointer_focus_(NULL),
    cursor_(NULL),
    keyboard_focus_(NULL),
    keyboard_modifiers_(0)
{
  static const struct wl_seat_listener kInputSeatListener = {
    WaylandInputDevice::OnSeatCapabilities,
  };

  input_seat_ = static_cast<wl_seat*>(
      wl_registry_bind(display->registry(), id, &wl_seat_interface, 1));
  wl_seat_add_listener(input_seat_, &kInputSeatListener, this);
  wl_seat_set_user_data(input_seat_, this);

  InitXKB();
}

WaylandInputDevice::~WaylandInputDevice()
{
  if (cursor_)
    delete cursor_;

  if (input_seat_)
    wl_seat_destroy(input_seat_);

  FiniXKB();
}

void WaylandInputDevice::InitXKB()
{
  xkb_.context = xkb_context_new((xkb_context_flags)0);
  if (!xkb_.context) {
    return;
  }
}

void WaylandInputDevice::FiniXKB()
{
  if (xkb_.state)
    xkb_state_unref(xkb_.state);
  if (xkb_.keymap)
    xkb_map_unref(xkb_.keymap);
  if (xkb_.context)
    xkb_context_unref(xkb_.context);
}

void WaylandInputDevice::OnSeatCapabilities(void *data, wl_seat *seat, uint32_t caps)
{
  WaylandInputDevice* device = static_cast<WaylandInputDevice*>(data);

  static const struct wl_pointer_listener kInputPointerListener = {
    WaylandInputDevice::OnPointerEnter,
    WaylandInputDevice::OnPointerLeave,
    WaylandInputDevice::OnMotionNotify,
    WaylandInputDevice::OnButtonNotify,
    WaylandInputDevice::OnAxisNotify,
  };

  if (!device->cursor_)
    device->cursor_ = new WaylandCursor(WaylandDisplay::GetDisplay()->shm());

  if ((caps & WL_SEAT_CAPABILITY_POINTER) && !device->cursor_->GetInputPointer()) {
    wl_pointer* input_pointer = wl_seat_get_pointer(seat);
    device->cursor_->SetInputPointer(input_pointer);
    wl_pointer_set_user_data(input_pointer, device);
    wl_pointer_add_listener(input_pointer, &kInputPointerListener, device);
  } else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && device->cursor_->GetInputPointer()) {
    wl_pointer_destroy(device->cursor_->GetInputPointer());
    device->cursor_->SetInputPointer(NULL);
  }

  static const struct wl_keyboard_listener kInputKeyboardListener = {
    WaylandInputDevice::OnKeyboardKeymap,
    WaylandInputDevice::OnKeyboardEnter,
    WaylandInputDevice::OnKeyboardLeave,
    WaylandInputDevice::OnKeyNotify,
    WaylandInputDevice::OnKeyModifiers,
  };

  if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !device->input_keyboard_) {
    device->input_keyboard_ = wl_seat_get_keyboard(seat);
    wl_keyboard_set_user_data(device->input_keyboard_, device);
    wl_keyboard_add_listener(device->input_keyboard_, &kInputKeyboardListener,
        device);
  } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && device->input_keyboard_) {
    wl_keyboard_destroy(device->input_keyboard_);
    device->input_keyboard_ = NULL;
  }
}

void WaylandInputDevice::DispatchEventHelper(scoped_ptr<ui::Event> key) {
  base::MessagePumpOzone::Current()->Dispatch(key.get());
}

void WaylandInputDevice::DispatchEvent(scoped_ptr<ui::Event> event) {
  base::MessageLoop::current()->PostTask(
      FROM_HERE, base::Bind(&DispatchEventHelper, base::Passed(&event)));
}

void WaylandInputDevice::OnMotionNotify(void* data,
    wl_pointer* input_pointer,
    uint32_t time,
    wl_fixed_t sx_w,
    wl_fixed_t sy_w)
{
  WaylandInputDevice* device = static_cast<WaylandInputDevice*>(data);
  float sx = wl_fixed_to_double(sx_w);
  float sy = wl_fixed_to_double(sy_w);

  device->pointer_position_.SetPoint(sx, sy);

  scoped_ptr<MouseEvent> mouseev(new MouseEvent(
      ui::ET_MOUSE_MOVED,
      gfx::Point(sx, sy),
      gfx::Point(sx, sy),
      /* flags */ 0));

  DispatchEvent(mouseev.PassAs<ui::Event>());
}

void WaylandInputDevice::OnAxisNotify(void* data,
    wl_pointer* input_pointer,
    uint32_t time,
    uint32_t axis,
    int32_t value)
{
  int x_offset = 0, y_offset = 0;
  WaylandInputDevice* device = static_cast<WaylandInputDevice*>(data);
  const int delta = ui::MouseWheelEvent::kWheelDelta;

  switch (axis) {
  case WL_POINTER_AXIS_HORIZONTAL_SCROLL:
    x_offset = value > 0 ? -delta : delta;
    break;
  case WL_POINTER_AXIS_VERTICAL_SCROLL:
    y_offset = value > 0 ? -delta : delta;
    break;
  }

  MouseEvent mouseev(
      ui::ET_MOUSEWHEEL,
      device->pointer_position_,
      device->pointer_position_,
      /* flags */ 0);

  scoped_ptr<MouseWheelEvent> wheelev(new MouseWheelEvent(
      mouseev,
      x_offset,
      y_offset));

  DispatchEvent(wheelev.PassAs<ui::Event>());
}

void WaylandInputDevice::OnButtonNotify(void* data,
    wl_pointer* input_pointer,
    uint32_t serial,
    uint32_t time,
    uint32_t button,
    uint32_t state)
{
  WaylandInputDevice* device = static_cast<WaylandInputDevice*>(data);

  WaylandDisplay::GetDisplay()->SetSerial(serial);

  EventType type;
  if (state == WL_POINTER_BUTTON_STATE_PRESSED)
    type = ui::ET_MOUSE_PRESSED;
  else
    type = ui::ET_MOUSE_RELEASED;

  // TODO(vignatti): simultaneous clicks fail
  int flags = 0;
  if (button == BTN_LEFT)
    flags = ui::EF_LEFT_MOUSE_BUTTON;
  else if (button == BTN_RIGHT)
    flags = ui::EF_RIGHT_MOUSE_BUTTON;
  else if (button == BTN_MIDDLE)
    flags = ui::EF_MIDDLE_MOUSE_BUTTON;

  scoped_ptr<MouseEvent> mouseev(new MouseEvent(
      type,
      device->pointer_position_,
      device->pointer_position_,
      flags));

  DispatchEvent(mouseev.PassAs<ui::Event>());
}

void WaylandInputDevice::OnKeyNotify(void* data,
    wl_keyboard* input_keyboard,
    uint32_t serial,
    uint32_t time,
    uint32_t key,
    uint32_t state)
{
  WaylandInputDevice* device = static_cast<WaylandInputDevice*>(data);
  uint32_t code, num_syms;
  const xkb_keysym_t *syms;
  xkb_keysym_t sym;
  xkb_mod_mask_t mask;

  WaylandDisplay::GetDisplay()->SetSerial(serial);

  EventType type;
  if (state == WL_KEYBOARD_KEY_STATE_PRESSED)
    type = ET_KEY_PRESSED;
  else
    type = ET_KEY_RELEASED;

  code = key + 8;
  num_syms = xkb_key_get_syms(device->xkb_.state, code, &syms);
  if(num_syms == 1)
    sym = syms[0];
  else
    sym = XKB_KEY_NoSymbol;

  mask = xkb_state_serialize_mods(device->xkb_.state,
      (xkb_state_component)(XKB_STATE_DEPRESSED | XKB_STATE_LATCHED));
  device->keyboard_modifiers_ = 0;
  if (mask & device->xkb_.control_mask)
    device->keyboard_modifiers_ |= EF_CONTROL_DOWN;
  if (mask & device->xkb_.alt_mask)
    device->keyboard_modifiers_ |= EF_ALT_DOWN;
  if (mask & device->xkb_.shift_mask)
    device->keyboard_modifiers_ |= EF_SHIFT_DOWN;

  scoped_ptr<KeyEvent> keyev(new KeyEvent(
      type,
      ui::KeyboardCodeFromXKeysym(sym),
      device->keyboard_modifiers_,
      true));

  DispatchEvent(keyev.PassAs<ui::Event>());
}

void WaylandInputDevice::OnKeyModifiers(void *data, wl_keyboard *keyboard,
    uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
    uint32_t mods_locked, uint32_t group)
{
  WaylandInputDevice* device = static_cast<WaylandInputDevice*>(data);

  xkb_state_update_mask(device->xkb_.state, mods_depressed, mods_latched,
      mods_locked, 0, 0, group);
}

void WaylandInputDevice::OnPointerEnter(void* data,
    wl_pointer* input_pointer,
    uint32_t serial,
    wl_surface* surface,
    wl_fixed_t sx_w,
    wl_fixed_t sy_w)
{
  WaylandInputDevice* device = static_cast<WaylandInputDevice*>(data);
  WaylandWindow* window;
  float sx = wl_fixed_to_double(sx_w);
  float sy = wl_fixed_to_double(sy_w);

  device->pointer_position_.SetPoint(sx, sy);

  WaylandDisplay::GetDisplay()->SetSerial(serial);
  device->pointer_enter_serial_ = serial;

  // If we have a surface, then a new window is in focus
  window = static_cast<WaylandWindow*>(wl_surface_get_user_data(surface));
  device->pointer_focus_ = window;

  // TODO(vignatti): sx and sy have to be used for setting different resizing
  // and other cursors.

  device->cursor_->Update(cursorType(WINDOW_TITLEBAR), serial);
  scoped_ptr<MouseEvent> mouseev(new MouseEvent(
      ui::ET_MOUSE_ENTERED,
      device->pointer_position_,
      device->pointer_position_,
      /* flags */ 0));

  DispatchEvent(mouseev.PassAs<ui::Event>()); 
}

void WaylandInputDevice::OnPointerLeave(void* data,
    wl_pointer* input_pointer,
    uint32_t serial,
    wl_surface* surface)
{
  WaylandInputDevice* device = static_cast<WaylandInputDevice*>(data);
  WaylandWindow* window = device->pointer_focus_;

  WaylandDisplay::GetDisplay()->SetSerial(serial);

  device->pointer_focus_ = NULL;

  scoped_ptr<MouseEvent> mouseev(new MouseEvent(
      ui::ET_MOUSE_EXITED,
      device->pointer_position_,
      device->pointer_position_,
      /* flags */ 0));

  DispatchEvent(mouseev.PassAs<ui::Event>());
}

void WaylandInputDevice::OnKeyboardKeymap(void *data,
    struct wl_keyboard *keyboard,
    uint32_t format, int fd, uint32_t size)
{
  WaylandInputDevice* device = static_cast<WaylandInputDevice*>(data);
  char *map_str;

  if (!data) {
    close(fd);
    return;
  }

  if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
    close(fd);
    return;
  }

  map_str = (char*) mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
  if (map_str == MAP_FAILED) {
    close(fd);
    return;
  }

  device->xkb_.keymap = xkb_map_new_from_string(device->xkb_.context,
      map_str, XKB_KEYMAP_FORMAT_TEXT_V1, (xkb_map_compile_flags)0);
  munmap(map_str, size);
  close(fd);
  if (!device->xkb_.keymap) {
    return;
  }

  device->xkb_.state = xkb_state_new(device->xkb_.keymap);
  if (!device->xkb_.state) {
    xkb_map_unref(device->xkb_.keymap);
    device->xkb_.keymap = NULL;
    return;
  }

  device->xkb_.control_mask =
    1 << xkb_map_mod_get_index(device->xkb_.keymap, "Control");
  device->xkb_.alt_mask =
    1 << xkb_map_mod_get_index(device->xkb_.keymap, "Mod1");
  device->xkb_.shift_mask =
    1 << xkb_map_mod_get_index(device->xkb_.keymap, "Shift");
}

void WaylandInputDevice::OnKeyboardEnter(void* data,
    wl_keyboard* input_keyboard,
    uint32_t serial,
    wl_surface* surface,
    wl_array* keys)
{
  WaylandInputDevice* device = static_cast<WaylandInputDevice*>(data);
  WaylandWindow* window;

  WaylandDisplay::GetDisplay()->SetSerial(serial);
  window = device->keyboard_focus_ =
    static_cast<WaylandWindow*>(wl_surface_get_user_data(surface));

#if 0
  WaylandEvent event;
  event.type = WAYLAND_KEYBOARD_FOCUS;
  event.keyboard_focus.serial = serial;
  device->keyboard_modifiers_ = 0;
  event.keyboard_focus.modifiers = device->keyboard_modifiers_;
  event.keyboard_focus.state = 1;

  if (!window->delegate())
    return;

  window->delegate()->OnKeyboardEnter(&event);
#endif
}

void WaylandInputDevice::OnKeyboardLeave(void* data,
    wl_keyboard* input_keyboard,
    uint32_t serial,
    wl_surface* surface)
{
  WaylandInputDevice* device = static_cast<WaylandInputDevice*>(data);
  WaylandWindow* window = device->keyboard_focus_;

  WaylandDisplay::GetDisplay()->SetSerial(serial);
#if 0
  WaylandEvent event;
  event.type = WAYLAND_KEYBOARD_FOCUS;
  event.keyboard_focus.serial = serial;
  device->keyboard_modifiers_ = 0;

  // If there is a window, then it loses focus
  if (window) {
    if(!WaylandDisplay::GetDisplay()->IsWindow(window))
      return;

    event.keyboard_focus.state = 0;
    device->keyboard_focus_ = NULL;

    if (!window->delegate())
      return;

    window->delegate()->OnKeyboardLeave(&event);
  }
#endif
}

}  // namespace ui
