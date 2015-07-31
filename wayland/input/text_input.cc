// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/input/text_input.h"

#include <string>

#include "ozone/wayland/display.h"
#include "ozone/wayland/input/keyboard.h"
#include "ozone/wayland/protocol/text-client-protocol.h"
#include "ozone/wayland/seat.h"
#include "ozone/wayland/shell/shell_surface.h"
#include "ozone/wayland/window.h"

namespace ozonewayland {

WaylandTextInput::WaylandTextInput(WaylandSeat* seat):
  text_input_(NULL), active_window_(NULL), last_active_window_(NULL),
  seat_(seat) {
  enable_vkb_support_ = getenv("USE_OZONE_WAYLAND_VKB");
}

WaylandTextInput::~WaylandTextInput() {
  if (text_input_)
    wl_text_input_destroy(text_input_);
}

void WaylandTextInput::SetActiveWindow(WaylandWindow* window) {
  active_window_ = window;
  if (active_window_)
    last_active_window_ = active_window_;
}

void WaylandTextInput::OnCommitString(void* data,
                                      struct wl_text_input* text_input,
                                      uint32_t serial,
                                      const char* text) {
  WaylandDisplay* dispatcher = WaylandDisplay::GetInstance();
  DCHECK(static_cast<WaylandTextInput*>(data)->last_active_window_);
  dispatcher->Commit(static_cast<WaylandTextInput*>(data)->
      last_active_window_->Handle(), std::string(text));
}

void WaylandTextInput::OnPreeditString(void* data,
                                       struct wl_text_input* text_input,
                                       uint32_t serial,
                                       const char* text,
                                       const char* commit) {
  WaylandDisplay* dispatcher = WaylandDisplay::GetInstance();
  DCHECK(static_cast<WaylandTextInput*>(data)->last_active_window_);
  dispatcher->PreeditChanged(static_cast<WaylandTextInput*>(data)->
     last_active_window_->Handle(), std::string(text), std::string(commit));
}

void WaylandTextInput::OnDeleteSurroundingText(void* data,
                                       struct wl_text_input* text_input,
                                       int32_t index,
                                       uint32_t length) {
}

void WaylandTextInput::OnCursorPosition(void* data,
                                       struct wl_text_input* text_input,
                                       int32_t index,
                                       int32_t anchor) {
}

void WaylandTextInput::OnPreeditStyling(void* data,
                                       struct wl_text_input* text_input,
                                       uint32_t index,
                                       uint32_t length,
                                       uint32_t style) {
}

void WaylandTextInput::OnPreeditCursor(void* data,
                                       struct wl_text_input* text_input,
                                       int32_t index) {
}

void WaylandTextInput::OnModifiersMap(void* data,
                                      struct wl_text_input* text_input,
                                      struct wl_array* map) {
}

void WaylandTextInput::OnKeysym(void* data,
                                struct wl_text_input* text_input,
                                uint32_t serial,
                                uint32_t time,
                                uint32_t key,
                                uint32_t state,
                                uint32_t modifiers) {
  // Copied from WaylandKeyboard::OnKeyNotify().
  ui::EventType type = ui::ET_KEY_PRESSED;
  WaylandDisplay::GetInstance()->SetSerial(serial);
  if (state == WL_KEYBOARD_KEY_STATE_RELEASED)
    type = ui::ET_KEY_RELEASED;
  WaylandDisplay* dispatcher = WaylandDisplay::GetInstance();
  const uint32_t device_id = wl_proxy_get_id(
      reinterpret_cast<wl_proxy*>(text_input));
  dispatcher->VirtualKeyNotify(type, key, device_id);
}

void WaylandTextInput::OnEnter(void* data,
                               struct wl_text_input* text_input,
                               struct wl_surface* surface) {
}

void WaylandTextInput::OnLeave(void* data,
                               struct wl_text_input* text_input) {
}

void WaylandTextInput::OnInputPanelState(void* data,
                               struct wl_text_input* text_input,
                               uint32_t state) {
}

void WaylandTextInput::OnLanguage(void* data,
                               struct wl_text_input* text_input,
                               uint32_t serial,
                               const char* language) {
}

void WaylandTextInput::OnTextDirection(void* data,
                               struct wl_text_input* text_input,
                               uint32_t serial,
                               uint32_t direction) {
}

void WaylandTextInput::ResetIme() {
  static const struct wl_text_input_listener text_input_listener = {
      WaylandTextInput::OnEnter,
      WaylandTextInput::OnLeave,
      WaylandTextInput::OnModifiersMap,
      WaylandTextInput::OnInputPanelState,
      WaylandTextInput::OnPreeditString,
      WaylandTextInput::OnPreeditStyling,
      WaylandTextInput::OnPreeditCursor,
      WaylandTextInput::OnCommitString,
      WaylandTextInput::OnCursorPosition,
      WaylandTextInput::OnDeleteSurroundingText,
      WaylandTextInput::OnKeysym,
      WaylandTextInput::OnLanguage,
      WaylandTextInput::OnTextDirection
  };

  if (!text_input_ && enable_vkb_support_) {
    text_input_ = wl_text_input_manager_create_text_input(
        WaylandDisplay::GetInstance()->GetTextInputManager());
    wl_text_input_add_listener(text_input_, &text_input_listener, this);
  }
}

void WaylandTextInput::ShowInputPanel(wl_seat* input_seat) {
  if (text_input_ && active_window_ && active_window_->ShellSurface()) {
    wl_text_input_show_input_panel(text_input_);
    wl_text_input_activate(text_input_,
                           input_seat,
                           active_window_->ShellSurface()->GetWLSurface());
  }
}

void WaylandTextInput::HideInputPanel(wl_seat* input_seat) {
  if (text_input_)
    wl_text_input_deactivate(text_input_, input_seat);
}

}  // namespace ozonewayland
