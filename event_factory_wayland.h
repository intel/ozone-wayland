// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_EVENT_FACTORY_H_
#define OZONE_WAYLAND_EVENT_FACTORY_H_

#include "ui/base/ui_export.h"
#include "base/event_types.h"
#include "base/message_loop/message_pump_libevent.h"
#include "base/native_library.h"

namespace ui {

class WaylandDisplay;

class EventFactoryWayland : public base::MessagePumpLibevent::Watcher {
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

  ui::WaylandDisplay* display_;
  int fd_;
  base::MessagePumpLibevent::FileDescriptorWatcher watcher_;
};

}  // namespace ui

#endif  // OZONE_WAYLAND_EVENT_FACTORY_H_
