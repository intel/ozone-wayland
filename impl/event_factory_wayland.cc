// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/event_factory_wayland.h"

#include "base/lazy_instance.h"
#include "base/memory/scoped_ptr.h"
#include "ozone/wayland/display.h"

namespace ui {

// Implementation.
static base::LazyInstance<scoped_ptr<EventFactoryWayland> > impl_ =
    LAZY_INSTANCE_INITIALIZER;

EventFactoryWayland::EventFactoryWayland()
    : fd_(-1) {
  LOG(INFO) << "Ozone: EventFactoryWayland";
  WaylandDisplay* dis = WaylandDisplay::GetDisplay();
  fd_ = wl_display_get_fd(dis->display());

  CHECK_GE(fd_, 0);
  bool success = base::MessagePumpOzone::Current()->WatchFileDescriptor(
                            fd_,
                            true,
                            base::MessagePumpLibevent::WATCH_READ,
                            &watcher_,
                            this);
  CHECK(success);

  base::MessageLoop::current()->AddTaskObserver(this);
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
  WaylandDisplay::GetDisplay()->Flush();
}

void EventFactoryWayland::OnFileCanWriteWithoutBlocking(int fd) {
  NOTREACHED();
}

void EventFactoryWayland::WillProcessTask(
    const base::PendingTask& pending_task) {
}

void EventFactoryWayland::DidProcessTask(
    const base::PendingTask& pending_task) {

  // a proper integration of libwayland should call wl_display_flush() only
  // before the client's event loop is about to go sleep. Ideally libevent
  // would emit a signal mentioning its intent to sleep and libwayland would
  // flush the remaining buffered bytes.
  // The catch with this hack in DidProcessTask is to be careful and flush only
  // when needed like after eglSwapBuffers (PostSwapBuffersComplete) and others
  // This would not be needed after we start using transport surface, as nested
  // server would be responsible for flushing the client as needed. We need to
  // come back to this once we have it working.

  if (strcmp(pending_task.posted_from.function_name(),
      "PostSwapBuffersComplete") != 0)
    return;

  WaylandDisplay::GetDisplay()->Flush();
}

}  // namespace ui
