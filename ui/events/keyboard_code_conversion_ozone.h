// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_EVENTS_KEYBOARD_CODE_CONVERSION_OZONE_H_
#define OZONE_UI_EVENTS_KEYBOARD_CODE_CONVERSION_OZONE_H_

#include "base/basictypes.h"
#include "ozone/platform/ozone_export_wayland.h"
#include "ozone/ui/events/keyboard_codes_ozone.h"

namespace ui {

  // A helper function to convert Ozone-WL types to ui::KeyboardCode type.
  OZONE_WAYLAND_EXPORT KeyboardCode KeyboardCodeFromNativeKeysym(unsigned key);
  OZONE_WAYLAND_EXPORT uint16 CharacterCodeFromNativeKeySym(unsigned sym,
                                                            unsigned flags);

}  // namespace ui

#endif  // OZONE_UI_EVENTS_KEYBOARD_CODE_CONVERSION_OZONE_H_
