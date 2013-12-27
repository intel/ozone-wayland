// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/input/keyboard.h"

#include "ozone/ui/ime/keyboard_engine_xkb.h"
#include "ozone/wayland/dispatcher.h"

namespace ozonewayland {

WaylandKeyboard::WaylandKeyboard() : input_keyboard_(NULL),
    dispatcher_(NULL),
    backend_(NULL) {
}

WaylandKeyboard::~WaylandKeyboard() {
  if (backend_)
    delete backend_;

  if (input_keyboard_) {
    wl_keyboard_destroy(input_keyboard_);
    input_keyboard_ = NULL;
  }
}

void WaylandKeyboard::OnSeatCapabilities(wl_seat *seat, uint32_t caps) {
  static const struct wl_keyboard_listener kInputKeyboardListener = {
    WaylandKeyboard::OnKeyboardKeymap,
    WaylandKeyboard::OnKeyboardEnter,
    WaylandKeyboard::OnKeyboardLeave,
    WaylandKeyboard::OnKeyNotify,
    WaylandKeyboard::OnKeyModifiers,
  };

  dispatcher_ = WaylandDispatcher::GetInstance();

  if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !input_keyboard_) {
    DCHECK(!backend_);
    backend_ =  new KeyboardEngineXKB();
    input_keyboard_ = wl_seat_get_keyboard(seat);
    wl_keyboard_add_listener(input_keyboard_, &kInputKeyboardListener,
        this);
  } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && input_keyboard_) {
    if (backend_) {
      delete backend_;
      backend_ = NULL;
    }

    wl_keyboard_destroy(input_keyboard_);
    input_keyboard_ = NULL;
  }
}

void WaylandKeyboard::OnKeyNotify(void* data,
                                  wl_keyboard* input_keyboard,
                                  uint32_t serial,
                                  uint32_t time,
                                  uint32_t key,
                                  uint32_t state) {
  WaylandKeyboard* device = static_cast<WaylandKeyboard*>(data);
  unsigned currentState = 1;
  WaylandDisplay::GetInstance()->SetSerial(serial);

  if (state == WL_KEYBOARD_KEY_STATE_PRESSED)
    currentState = 1;
  else
    currentState = 0;

  // Check if we can ignore the KeyEvent notification, saves an IPC call.
  if (device->backend_->IgnoreKeyNotify(key, currentState))
    return;

  device->dispatcher_->KeyNotify(currentState,
                                 device->backend_->ConvertKeyCodeFromEvdev(key),
                                 device->backend_->GetKeyBoardModifiers());
}

void WaylandKeyboard::OnKeyboardKeymap(void *data,
                                       struct wl_keyboard *keyboard,
                                       uint32_t format,
                                       int fd,
                                       uint32_t size) {
  WaylandKeyboard* device = static_cast<WaylandKeyboard*>(data);

  if (!data) {
    close(fd);
    return;
  }

  if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
    close(fd);
    return;
  }

  device->backend_->OnKeyboardKeymap(fd, size);
  close(fd);
}

void WaylandKeyboard::OnKeyboardEnter(void* data,
                                      wl_keyboard* input_keyboard,
                                      uint32_t serial,
                                      wl_surface* surface,
                                      wl_array* keys) {
  WaylandDisplay::GetInstance()->SetSerial(serial);
}

void WaylandKeyboard::OnKeyboardLeave(void* data,
                                      wl_keyboard* input_keyboard,
                                      uint32_t serial,
                                      wl_surface* surface) {
  WaylandDisplay::GetInstance()->SetSerial(serial);
}

void WaylandKeyboard::OnKeyModifiers(void *data,
                                     wl_keyboard *keyboard,
                                     uint32_t serial,
                                     uint32_t mods_depressed,
                                     uint32_t mods_latched,
                                     uint32_t mods_locked,
                                     uint32_t group) {
  WaylandKeyboard* device = static_cast<WaylandKeyboard*>(data);
  device->backend_->OnKeyModifiers(mods_depressed,
                                   mods_latched,
                                   mods_locked,
                                   group);
}

}  // namespace ozonewayland
