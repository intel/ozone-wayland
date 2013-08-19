// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland_task.h"

#include <wayland-client.h>

#include "ozone/wayland_display.h"
#include "ozone/wayland_window.h"

namespace ui {

////////////////// WaylandTask Implementation ////////////////////////////
WaylandTask::WaylandTask(WaylandWindow *window) : window_(window)
{
}

WaylandTask::~WaylandTask()
{
}

////////////////// WaylandResizeTask Implementation ////////////////////////////
WaylandResizeTask::WaylandResizeTask(WaylandWindow *window) : WaylandTask(window)
{
}

WaylandResizeTask::~WaylandResizeTask()
{
}

void WaylandResizeTask::Run()
{
  window_->OnResize();
}

}  // namespace ui
