// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/vsync_provider_wayland.h"

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "ozone/impl/ozone_display.h"
#include "ozone/ui/events/window_state_change_handler.h"
#include "ozone/wayland/display.h"

namespace ozonewayland {

WaylandSyncProvider::WaylandSyncProvider(unsigned handle) : handle_(handle) {
}

WaylandSyncProvider::~WaylandSyncProvider() {
  // WaylandSyncProvider is destroyed when the associated NativeSurface is
  // destroyed. Inform OzoneDisplay to release Wayland resources.
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(handle_,
                                                              ui::DESTROYED);
  WaylandDisplay::GetInstance()->FlushDisplay();
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
  WaylandDisplay* native_display = WaylandDisplay::GetInstance();
  if (native_display)
    native_display->FlushDisplay();
}

}  // namespace ozonewayland
