// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/dispatcher.h"
#include "base/bind.h"
#include "base/message_loop/message_pump_ozone.h"
#include <stdio.h>

namespace ui {

WaylandDispatcher::WaylandDispatcher()
    : Thread("WaylandDispatcher"),
      ignore_task_(false)
{
  loop_ = base::MessageLoop::current();
  Options options;
  options.message_loop_type = base::MessageLoop::TYPE_IO;
  StartWithOptions(options);
  SetPriority(base::kThreadPriority_Background);
}

void WaylandDispatcher::DispatchEventHelper(scoped_ptr<ui::Event> key) {
  base::MessagePumpOzone::Current()->Dispatch(key.get());
}

void WaylandDispatcher::PostTask(Task type)
{
  if (!IsRunning() || ignore_task_)
    return;

  switch(type) {
    case (Flush):
      message_loop_proxy()->PostTask(
                  FROM_HERE, base::Bind(&WaylandDispatcher::HandleFlush));
      break;
    default:
      break;
  }
}

void WaylandDispatcher::MessageLoopDestroyed()
{
  if (!IsRunning())
    return;

  ignore_task_ = true;
  loop_ = NULL;
  Stop();
}

void WaylandDispatcher::PostTaskOnMainLoop(
        const tracked_objects::Location& from_here, const base::Closure& task)
{
  if (ignore_task_ || !IsRunning() || !loop_)
    return;

  loop_->message_loop_proxy()->PostTask(from_here, task);
}

void WaylandDispatcher::DispatchEvent(scoped_ptr<ui::Event> event) {
  PostTaskOnMainLoop(FROM_HERE, base::Bind(&DispatchEventHelper,
                                           base::Passed(&event)));
}

WaylandDispatcher::~WaylandDispatcher()
{
  ignore_task_ = true;
  loop_ = NULL;
  Stop();
}

void WaylandDispatcher::HandleFlush()
{
  WaylandDisplay* display = WaylandDisplay::GetDisplay();
  if (!display)
    return;

  wl_display* waylandDisp = display->display();
  if (!waylandDisp)
    return;

  while (wl_display_prepare_read(waylandDisp) != 0)
    wl_display_dispatch_pending(waylandDisp);

  wl_display_flush(waylandDisp);
  wl_display_read_events(waylandDisp);
  wl_display_dispatch_pending(waylandDisp);
}

}  // namespace ui
