// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_DESKTOP_AURA_OZONE_UTIL_H_
#define OZONE_IMPL_DESKTOP_AURA_OZONE_UTIL_H_

#include "base/basictypes.h"

#include "ui/views/views_export.h"

namespace ui {

// Determine whether we should default to native decorations or the custom
// frame based on the currently-running window manager.
VIEWS_EXPORT bool GetCustomFramePrefDefault();

}  // namespace ui

#endif  // OZONE_IMPL_DESKTOP_AURA_OZONE_UTIL_H_
