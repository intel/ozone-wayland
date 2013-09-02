// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_DISPATCHER_H_
#define OZONE_WAYLAND_DISPATCHER_H_

#include "base/threading/thread.h"
#include "base/memory/scoped_ptr.h"
#include "ui/base/events/event.h"
#include "ozone/wayland/display.h"

namespace ui {
class EventFactoryWayland;
class WaylandDisplay;

class WaylandDispatcher : public base::Thread {
public:
  enum Task
  {
    Flush = 0x01
  };

  WaylandDispatcher(WaylandDisplay* display);
  virtual ~WaylandDispatcher();

  void PostTask(Task type = Flush);
  void DispatchEvent(scoped_ptr<ui::Event> event);
  void PostTaskOnMainLoop(const tracked_objects::Location& from_here, const base::Closure& task);

private:
  static void HandleFlush();
  static void DispatchEventHelper(scoped_ptr<ui::Event> key);
  void StopPolling();
  static void pollEventLoop(WaylandDispatcher* data);
  base::MessageLoop* loop_;
  bool ignore_task_ :1;
  friend class EventFactoryWayland;
  DISALLOW_COPY_AND_ASSIGN(WaylandDispatcher);
};

}  // namespace ui

#endif  // OZONE_WAYLAND_DISPATCHER_H_

