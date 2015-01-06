// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/events/keyboard_engine_xkb.h"

#include <sys/mman.h>

#include "base/strings/stringprintf.h"
#include "ozone/ui/events/keyboard_codes_ozone.h"
#include "ui/events/event.h"
#include "ui/events/ozone/layout/keyboard_layout_engine_manager.h"

namespace ui {

KeyboardEngineXKB::KeyboardEngineXKB(const XkbKeyCodeConverter& converter)
    : XkbKeyboardLayoutEngine(converter),
    context_(NULL) {
}

KeyboardEngineXKB::~KeyboardEngineXKB() {
  //FiniXKB();
}

bool KeyboardEngineXKB::SetCurrentLayoutByName(const std::string& layout_name) {
  return XkbKeyboardLayoutEngine::SetCurrentLayoutByName(layout_name);
}

void KeyboardEngineXKB::InitXKB() {
  if (context_)
    return;

  context_ = xkb_context_new((xkb_context_flags)0);
}

void KeyboardEngineXKB::FiniXKB() {
  if (context_) {
    xkb_context_unref(context_);
    context_ = NULL;
  }
}

}  // namespace ui
