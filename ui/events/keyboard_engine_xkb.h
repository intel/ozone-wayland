// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_EVENTS_INPUT_KEYBOARD_ENGINE_XKB_H_
#define OZONE_UI_EVENTS_INPUT_KEYBOARD_ENGINE_XKB_H_

#include <xkbcommon/xkbcommon.h>
#include <string>

#include "base/basictypes.h"
#include "ozone/platform/ozone_export_wayland.h"
#include "ozone/ui/events/keyboard_codes_ozone.h"
#include "ui/events/ozone/layout/xkb/xkb_keyboard_layout_engine.h"

namespace ui {

class KeyboardEngineXKB : public XkbKeyboardLayoutEngine {
 public:
  explicit KeyboardEngineXKB(const XkbKeyCodeConverter& converter);
  ~KeyboardEngineXKB() override;

  bool SetCurrentLayoutByName(const std::string& layout_name) override;

 private:
  void InitXKB();
  void FiniXKB();
  struct xkb_context *context_;

  DISALLOW_COPY_AND_ASSIGN(KeyboardEngineXKB);
};

}  // namespace ui

#endif  // OZONE_UI_EVENTS_INPUT_KEYBOARD_ENGINE_XKB_H_
