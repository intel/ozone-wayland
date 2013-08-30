// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Copyright (C) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_KBD_CONVERSION_H_
#define OZONE_WAYLAND_KBD_CONVERSION_H_

#include "base/basictypes.h"
#include "ui/base/keycodes/keyboard_codes_posix.h"
#include "ui/base/ui_export.h"

namespace ui {

UI_EXPORT KeyboardCode KeyboardCodeFromXKeysym(unsigned int keysym);

}  // namespace ui

#endif  // OZONE_WAYLAND_KBD_CONVERSION_H_
