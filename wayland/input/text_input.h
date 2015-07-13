// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_TEXT_INPUT_H_
#define OZONE_WAYLAND_TEXT_INPUT_H_

#include "ozone/wayland/display.h"

struct wl_text_input;

namespace ozonewayland {

class WaylandSeat;
class WaylandWindow;

class WaylandTextInput {
 public:
  explicit WaylandTextInput(WaylandSeat* inputDevice);
  ~WaylandTextInput();
  void ResetIme();
  void ShowInputPanel(wl_seat* input_seat);
  void HideInputPanel(wl_seat* input_seat);
  void SetActiveWindow(WaylandWindow* window);
  WaylandSeat* getSeat() { return seat_; }

 private:
  static void OnCommitString(void* data,
                             struct wl_text_input* text_input,
                             uint32_t serial,
                             const char* text);

  static void OnPreeditString(void* data,
                             struct wl_text_input* text_input,
                             uint32_t serial,
                             const char* text,
                             const char* commit);

  static void OnDeleteSurroundingText(void* data,
                             struct wl_text_input* text_input,
                             int32_t index,
                             uint32_t length);

  static void OnCursorPosition(void* data,
                             struct wl_text_input* text_input,
                             int32_t index,
                             int32_t anchor);

  static void OnPreeditStyling(void* data,
                             struct wl_text_input* text_input,
                             uint32_t index,
                             uint32_t length,
                             uint32_t style);

  static void OnPreeditCursor(void* data,
                            struct wl_text_input* text_input,
                            int32_t index);

  static void OnModifiersMap(void* data,
                            struct wl_text_input* text_input,
                            struct wl_array* map);

  static void OnKeysym(void* data,
                       struct wl_text_input* text_input,
                       uint32_t serial,
                       uint32_t time,
                       uint32_t key,
                       uint32_t state,
                       uint32_t modifiers);

  static void OnEnter(void* data,
                      struct wl_text_input* text_input,
                      struct wl_surface* surface);

  static void OnLeave(void* data,
                      struct wl_text_input* text_input);

  static void OnInputPanelState(void* data,
                      struct wl_text_input* text_input,
                      uint32_t state);

  static void OnLanguage(void* data,
                      struct wl_text_input* text_input,
                      uint32_t serial,
                      const char* language);

  static void OnTextDirection(void* data,
                      struct wl_text_input* text_input,
                      uint32_t serial,
                      uint32_t direction);
  bool enable_vkb_support_;
  struct wl_text_input* text_input_;
  WaylandWindow* active_window_;
  WaylandWindow* last_active_window_;
  WaylandSeat* seat_;

  DISALLOW_COPY_AND_ASSIGN(WaylandTextInput);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_TEXT_INPUT_H_
