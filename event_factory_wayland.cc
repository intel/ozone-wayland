// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/event_factory_wayland.h"

#include "base/lazy_instance.h"
#include "base/memory/scoped_ptr.h"
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

  base::MessageLoop::current()->AddTaskObserver(this);
}

EventFactoryWayland::~EventFactoryWayland() {
  base::MessageLoop::current()->RemoveTaskObserver(this);
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

  if (strcmp(pending_task.posted_from.function_name(),
      "PostSwapBuffersComplete") != 0)
    return;

  wl_display_dispatch_pending(display_->display());
  wl_display_flush(display_->display());
}

}  // namespace ui
