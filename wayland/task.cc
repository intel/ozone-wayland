// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/task.h"

#include <wayland-client.h>

#include "ozone/wayland/display.h"
#include "ozone/wayland/window.h"

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
