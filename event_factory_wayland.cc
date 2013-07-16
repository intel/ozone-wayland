// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/event_factory_wayland.h"

#include "base/lazy_instance.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_pump_ozone.h"
#include "ozone/wayland_display.h"

namespace ui {

// Implementation.
static base::LazyInstance<scoped_ptr<EventFactoryWayland> > impl_ =
    LAZY_INSTANCE_INITIALIZER;

EventFactoryWayland::EventFactoryWayland()
    : fd_(-1) {
  LOG(INFO) << "Ozone: EventFactoryWayland";

  display_ = ui::WaylandDisplay::GetDisplay();
  fd_ = wl_display_get_fd(display_->display());

  CHECK_GE(fd_, 0);
  bool success = base::MessagePumpOzone::Current()->WatchFileDescriptor(
                            fd_,
                            true,
                            base::MessagePumpLibevent::WATCH_READ,
                            &watcher_,
                            this);
  CHECK(success);

  display_->ProcessTasks();
  wl_display_dispatch_pending(display_->display());
  wl_display_flush(display_->display());
}

EventFactoryWayland::~EventFactoryWayland() {
  watcher_.StopWatchingFileDescriptor();
}

EventFactoryWayland* EventFactoryWayland::GetInstance() {
  return impl_.Get().get();
}

void EventFactoryWayland::SetInstance(EventFactoryWayland* impl) {
  impl_.Get().reset(impl);
}

void EventFactoryWayland::OnFileCanReadWithoutBlocking(int fd) {
  display_->ProcessTasks();

  while (wl_display_prepare_read(display_->display()) != 0)
    wl_display_dispatch_pending(display_->display());
  wl_display_flush(display_->display());

  wl_display_read_events(display_->display());
  wl_display_dispatch_pending(display_->display());
}

void EventFactoryWayland::OnFileCanWriteWithoutBlocking(int fd) {
  NOTREACHED();
}

}  // namespace ui
