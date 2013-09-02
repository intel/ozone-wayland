// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/keyboard.h"

#include "ozone/wayland/dispatcher.h"
#include "ozone/wayland/kbd_conversion.h"
#include "ui/base/events/event.h"
#include <sys/mman.h>

namespace ui {

KeyBoard::KeyBoard()
  : input_keyboard_(NULL),
    keyboard_modifiers_(0)
{
  xkb_.context = NULL;
}

KeyBoard::~KeyBoard()
{
  FiniXKB();
  if (input_keyboard_) {
    wl_keyboard_destroy(input_keyboard_);
    input_keyboard_ = NULL;
  }
}

void KeyBoard::InitXKB()
{
  if (xkb_.context) {
    return;
  }

  xkb_.context = xkb_context_new((xkb_context_flags)0);
}

void KeyBoard::FiniXKB()
{
  if (xkb_.state)
    xkb_state_unref(xkb_.state);
  if (xkb_.keymap)
    xkb_map_unref(xkb_.keymap);
  if (xkb_.context)
    xkb_context_unref(xkb_.context);
}

void KeyBoard::OnSeatCapabilities(wl_seat *seat, uint32_t caps)
{
  static const struct wl_keyboard_listener kInputKeyboardListener = {
    KeyBoard::OnKeyboardKeymap,
    KeyBoard::OnKeyboardEnter,
    KeyBoard::OnKeyboardLeave,
    KeyBoard::OnKeyNotify,
    KeyBoard::OnKeyModifiers,
  };

  if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !input_keyboard_) {
    InitXKB();
    input_keyboard_ = wl_seat_get_keyboard(seat);
    wl_keyboard_add_listener(input_keyboard_, &kInputKeyboardListener,
        this);
  } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && input_keyboard_) {
    FiniXKB();
    wl_keyboard_destroy(input_keyboard_);
    input_keyboard_ = NULL;
  }
}

void KeyBoard::OnKeyNotify(void* data,
    wl_keyboard* input_keyboard,
    uint32_t serial,
    uint32_t time,
    uint32_t key,
    uint32_t state)
{
  KeyBoard* device = static_cast<KeyBoard*>(data);
  uint32_t code, num_syms;
  const xkb_keysym_t *syms;
  xkb_keysym_t sym;
  xkb_mod_mask_t mask;

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

  if (WaylandDisplay::GetDisplay()->Dispatcher())
    WaylandDisplay::GetDisplay()->Dispatcher()->DispatchEvent(keyev.PassAs<ui::Event>());
}

void KeyBoard::OnKeyModifiers(void *data, wl_keyboard *keyboard,
    uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
    uint32_t mods_locked, uint32_t group)
{
  KeyBoard* device = static_cast<KeyBoard*>(data);

  xkb_state_update_mask(device->xkb_.state, mods_depressed, mods_latched,
      mods_locked, 0, 0, group);
}

void KeyBoard::OnKeyboardKeymap(void *data,
    struct wl_keyboard *keyboard,
    uint32_t format, int fd, uint32_t size)
{
  KeyBoard* device = static_cast<KeyBoard*>(data);
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

void KeyBoard::OnKeyboardEnter(void* data,
    wl_keyboard* input_keyboard,
    uint32_t serial,
    wl_surface* surface,
    wl_array* keys)
{
}

void KeyBoard::OnKeyboardLeave(void* data,
    wl_keyboard* input_keyboard,
    uint32_t serial,
    wl_surface* surface)
{
}

}  // namespace ui

