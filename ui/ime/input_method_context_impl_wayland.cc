// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/ime/input_method_context_impl_wayland.h"

#include "base/logging.h"
#include "ozone/ui/ime/ime_state_change_handler.h"

namespace ozonewayland {

InputMethodContextImplWayland::InputMethodContextImplWayland(
    ui::LinuxInputMethodContextDelegate* delegate)
    : delegate_(delegate) {
  CHECK(delegate_);
}

InputMethodContextImplWayland::~InputMethodContextImplWayland() {
}

////////////////////////////////////////////////////////////////////////////////
// InputMethodContextImplWayland, ui::LinuxInputMethodContext implementation:
bool InputMethodContextImplWayland::DispatchKeyEvent(
    const ui::KeyEvent& key_event) {
  return false;
}

void InputMethodContextImplWayland::Reset() {
  IMEStateChangeHandler::GetInstance()->ResetIme();
}

void InputMethodContextImplWayland::OnTextInputTypeChanged(
    ui::TextInputType text_input_type) {
  // TODO(vignatti): this is recurrently called, disturbing the debug for being
  // very verbose; so that's why NOTIMPLEMENTED() isn't used here instead.
}

void InputMethodContextImplWayland::OnCaretBoundsChanged(
    const gfx::Rect& caret_bounds) {
  IMEStateChangeHandler::GetInstance()->ImeCaretBoundsChanged(caret_bounds);
}

}  // namespace ozonewayland
