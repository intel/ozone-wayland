// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/gfx/vsync_provider_wayland.h"

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "ozone/ui/gfx/ozone_display.h"

namespace gfx {

WaylandSyncProvider::WaylandSyncProvider() {
}

WaylandSyncProvider::~WaylandSyncProvider() {
}

void WaylandSyncProvider::GetVSyncParameters(
         const UpdateVSyncCallback& callback) {
  // TODO(kalyan): Pass refresh rate and time of the last refresh.
}

}  // namespace gfx
