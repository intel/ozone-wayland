// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/event_factory_wayland.h"

#include "ozone/wayland/dispatcher.h"
#include "ozone/wayland/display.h"
#include "base/lazy_instance.h"

namespace ozonewayland {

// Implementation.
static base::LazyInstance<scoped_ptr<EventFactoryWayland> > impl_ =
    LAZY_INSTANCE_INITIALIZER;

EventFactoryWayland::EventFactoryWayland()
    : fd_(-1) {
  LOG(INFO) << "Ozone: EventFactoryWayland";
  fd_ = wl_display_get_fd(WaylandDisplay::GetInstance()->display());
  CHECK_GE(fd_, 0);
  bool success = base::MessagePumpOzone::Current()->WatchFileDescriptor(
                            fd_,
                            true,
                            base::MessagePumpLibevent::WATCH_READ,
                            &watcher_,
                            this);

  CHECK(success);
  DCHECK(base::MessageLoop::current());
  base::MessageLoop::current()->AddTaskObserver(this);
  dispatcher_ = WaylandDispatcher::GetInstance();
}

EventFactoryWayland::~EventFactoryWayland() {
}

EventFactoryWayland* EventFactoryWayland::GetInstance() {
  return impl_.Get().get();
}

void EventFactoryWayland::SetInstance(EventFactoryWayland* impl) {
  impl_.Get().reset(impl);
}

void EventFactoryWayland::WillDestroyCurrentMessageLoop()
{
  watcher_.StopWatchingFileDescriptor();
  base::MessageLoop::current()->RemoveTaskObserver(this);
}

void EventFactoryWayland::OnFileCanReadWithoutBlocking(int fd) {
  dispatcher_->PostTask();
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

  dispatcher_->PostTask();
}

}  // namespace ozonewayland
