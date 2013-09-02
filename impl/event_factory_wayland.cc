// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/event_factory_wayland.h"

#include "base/lazy_instance.h"
#include "ozone/wayland/display.h"
#include "ozone/wayland/dispatcher.h"

namespace ui {

// Implementation.
static base::LazyInstance<scoped_ptr<EventFactoryWayland> > impl_ =
    LAZY_INSTANCE_INITIALIZER;

EventFactoryWayland::EventFactoryWayland()
    : fd_(-1) {
  LOG(INFO) << "Ozone: EventFactoryWayland";
  WaylandDisplay* dis = WaylandDisplay::GetDisplay();
  WaylandDispatcher::HandleFlush();

  fd_ = wl_display_get_fd(dis->display());

  CHECK_GE(fd_, 0);
  bool success = base::MessagePumpOzone::Current()->WatchFileDescriptor(
                            fd_,
                            true,
                            base::MessagePumpLibevent::WATCH_READ,
                            &watcher_,
                            this);
  CHECK(success);

  loop_ = base::MessageLoop::current();

  if (loop_)
    loop_->AddDestructionObserver(this);
}

EventFactoryWayland::~EventFactoryWayland() {
}

EventFactoryWayland* EventFactoryWayland::GetInstance() {
  return impl_.Get().get();
}

void EventFactoryWayland::SetInstance(EventFactoryWayland* impl) {
  impl_.Get().reset(impl);
}

void EventFactoryWayland::OnFileCanReadWithoutBlocking(int fd) {
  WaylandDisplay::GetDisplay()->Dispatcher()->PostTask();
}

void EventFactoryWayland::OnFileCanWriteWithoutBlocking(int fd) {
  NOTREACHED();
}

void EventFactoryWayland::WillDestroyCurrentMessageLoop()
{
  DCHECK(base::MessageLoop::current());
  if (loop_) {
    if (WaylandDisplay::GetDisplay() && WaylandDisplay::GetDisplay()->Dispatcher())
      WaylandDisplay::GetDisplay()->Dispatcher()->StopPolling();

    watcher_.StopWatchingFileDescriptor();
    base::MessageLoop::current()->RemoveDestructionObserver(this);
    loop_ = NULL;
  }
}

}  // namespace ui
