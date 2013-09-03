// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_EVENT_FACTORY_H_
#define OZONE_WAYLAND_EVENT_FACTORY_H_

#include "ui/base/ui_export.h"
#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_pump_libevent.h"

namespace ui {

class WaylandDisplay;

class EventFactoryWayland : public base::MessageLoop::TaskObserver,
                            public base::MessagePumpLibevent::Watcher,
                            public base::MessageLoop::DestructionObserver {
 public:
  EventFactoryWayland();
  virtual ~EventFactoryWayland();

  // Returns the instance
  UI_EXPORT static EventFactoryWayland* GetInstance();

  // Sets the implementation delegate.
  UI_EXPORT static void SetInstance(EventFactoryWayland* impl);

 private:
  // base::MessagePump:Libevent::Watcher implementation.
  virtual void OnFileCanReadWithoutBlocking(int fd) OVERRIDE;
  virtual void OnFileCanWriteWithoutBlocking(int fd) OVERRIDE;

  // MessageLoop::DestructionObserver overrides.
  virtual void WillDestroyCurrentMessageLoop() OVERRIDE;

  // Implements MessageLoop::TaskObserver.
  virtual void WillProcessTask(const base::PendingTask& pending_task) OVERRIDE;
  virtual void DidProcessTask(const base::PendingTask& pending_task) OVERRIDE;

  int fd_;
  base::MessagePumpLibevent::FileDescriptorWatcher watcher_;
};

}  // namespace ui

#endif  // OZONE_WAYLAND_EVENT_FACTORY_H_
