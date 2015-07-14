// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/webui/input_method_context_impl_wayland.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ozone/ui/events/ime_state_change_handler.h"
#include "ui/base/ime/composition_text.h"

namespace ui {

InputMethodContextImplWayland::InputMethodContextImplWayland(
    LinuxInputMethodContextDelegate* delegate)
    : delegate_(delegate) {
  CHECK(delegate_);
  ui::EventFactoryOzoneWayland::GetInstance()->SetIMEChangeObserver(this);
}

InputMethodContextImplWayland::~InputMethodContextImplWayland() {
}

////////////////////////////////////////////////////////////////////////////////
// InputMethodContextImplWayland, ui::LinuxInputMethodContext implementation:
bool InputMethodContextImplWayland::DispatchKeyEvent(
    const KeyEvent& key_event) {
  return false;
}

void InputMethodContextImplWayland::Reset() {
  EventFactoryOzoneWayland::GetInstance()->GetImeStateChangeHandler()->
      ResetIme();
}

void InputMethodContextImplWayland::Focus() {
}

void InputMethodContextImplWayland::Blur() {
}

void InputMethodContextImplWayland::SetCursorLocation(const gfx::Rect&) {
}

////////////////////////////////////////////////////////////////////////////////
// InputMethodContextImplWayland, ui::LinuxInputMethodContext implementation:

void InputMethodContextImplWayland::OnCommit(unsigned handle,
                                             const std::string& text) {
  delegate_->OnCommit(base::string16(base::ASCIIToUTF16(text.c_str())));
}

void InputMethodContextImplWayland::OnPreeditChanged(
    unsigned handle, const std::string& text, const std::string& commit) {
  ui::CompositionText composition_text;
  composition_text.text = base::string16(base::ASCIIToUTF16(text.c_str()));
  delegate_->OnPreeditChanged(composition_text);
}

}  // namespace ui
