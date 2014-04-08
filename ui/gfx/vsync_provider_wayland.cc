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
  // PassThroughImageTransportSurface calls this before swapping the surface.
  // Post a task to flush display as it should be handled immediately after
  // swap buffers is done.
  base::MessageLoop::current()->message_loop_proxy()->PostTask(
      FROM_HERE, base::Bind(&WaylandSyncProvider::ScheduleFlush));

  // TODO(kalyan): Pass refresh rate and time of the last refresh.
}

void WaylandSyncProvider::ScheduleFlush() {
  OzoneDisplay* native_display = OzoneDisplay::GetInstance();
  if (native_display)
    native_display->FlushDisplay();
}

}  // namespace gfx
