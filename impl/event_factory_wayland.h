// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_EVENT_FACTORY_WAYLAND_H_
#define OZONE_IMPL_EVENT_FACTORY_WAYLAND_H_

#include "ui/base/ui_export.h"
#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_pump_libevent.h"

namespace ozonewayland {

class WaylandDispatcher;

class EventFactoryWayland : public base::MessageLoop::TaskObserver,
                            public base::MessagePumpLibevent::Watcher {
 public:
  EventFactoryWayland();
  virtual ~EventFactoryWayland();

  // Returns the instance
  UI_EXPORT static EventFactoryWayland* GetInstance();

  // Sets the implementation delegate.
  UI_EXPORT static void SetInstance(EventFactoryWayland* impl);

  void WillDestroyCurrentMessageLoop();

 private:
  // base::MessagePump:Libevent::Watcher implementation.
  virtual void OnFileCanReadWithoutBlocking(int fd) OVERRIDE;
  virtual void OnFileCanWriteWithoutBlocking(int fd) OVERRIDE;

  // Implements MessageLoop::TaskObserver.
  virtual void WillProcessTask(const base::PendingTask& pending_task) OVERRIDE;
  virtual void DidProcessTask(const base::PendingTask& pending_task) OVERRIDE;

  int fd_;
  base::MessagePumpLibevent::FileDescriptorWatcher watcher_;
  WaylandDispatcher* dispatcher_;
};

}  // namespace ozonewayland

#endif  // OZONE_IMPL_EVENT_FACTORY_WAYLAND_H_
