// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/display_poll_thread.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <wayland-client.h>

#include "base/bind.h"
#include "ozone/wayland/display.h"

namespace ozonewayland {
const int MAX_EVENTS = 16;
// os-compatibility
extern "C" {
int osEpollCreateCloExec(void);

static int setCloExecOrClose(int fd) {
  int flags;

  if (fd == -1)
    return -1;

  flags = fcntl(fd, F_GETFD);
  if (flags == -1)
    goto err;

  if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
    goto err;

  return fd;

  err:
    close(fd);
    return -1;
}

int osEpollCreateCloExec(void) {
  int fd;

#ifdef EPOLL_CLOEXEC
  fd = epoll_create1(EPOLL_CLOEXEC);
  if (fd >= 0)
    return fd;
  if (errno != EINVAL)
    return -1;
#endif

  fd = epoll_create(1);
  return setCloExecOrClose(fd);
}
}  // os-compatibility

WaylandDisplayPollThread::WaylandDisplayPollThread(wl_display* display)
    : base::Thread("WaylandDisplayPollThread"),
      display_(display),
      polling_(true, false),
      stop_polling_(true, false) {
  DCHECK(display_);
  epoll_fd_ = osEpollCreateCloExec();
  if (epoll_fd_ < 0)
    LOG(ERROR) << "Epoll creation failed.";
}

WaylandDisplayPollThread::~WaylandDisplayPollThread() {
  StopProcessingEvents();
}

void WaylandDisplayPollThread::StartProcessingEvents() {
  DCHECK(!polling_.IsSignaled() && epoll_fd_);
  base::Thread::Options options;
  options.message_loop_type = base::MessageLoop::TYPE_IO;
  StartWithOptions(options);
  SetPriority(base::kThreadPriority_Background);
  message_loop_proxy()->PostTask(FROM_HERE, base::Bind(
      &WaylandDisplayPollThread::DisplayRun, this));
}

void WaylandDisplayPollThread::StopProcessingEvents() {
  if (polling_.IsSignaled())
    stop_polling_.Signal();

  Stop();
}

void WaylandDisplayPollThread::CleanUp() {
  SetThreadWasQuitProperly(true);
  if (epoll_fd_)
    close(epoll_fd_);
}

void  WaylandDisplayPollThread::DisplayRun(WaylandDisplayPollThread* data) {
  struct epoll_event ep[MAX_EVENTS];
  int i, ret, count = 0;
  uint32_t event = 0;
  bool epoll_err = false;
  unsigned display_fd = wl_display_get_fd(data->display_);
  int epoll_fd = data->epoll_fd_;
  ep[0].events = EPOLLIN;
  ep[0].data.ptr = 0;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, display_fd, &ep[0]) < 0) {
    close(epoll_fd);
    data->epoll_fd_ = 0;
    LOG(ERROR) << "epoll_ctl Add failed";
    return;
  }

  // Set the signal state. This is used to query from other threads (i.e.
  // StopProcessingEvents on Main thread), if this thread is still polling.
  data->polling_.Signal();

  // Adopted from:
  // http://cgit.freedesktop.org/wayland/weston/tree/clients/window.c#n5531.
  while (1) {
    wl_display_dispatch_pending(data->display_);
    ret = wl_display_flush(data->display_);
    if (ret < 0 && errno == EAGAIN) {
      ep[0].events = EPOLLIN | EPOLLERR | EPOLLHUP;
      epoll_ctl(epoll_fd, EPOLL_CTL_MOD, display_fd, &ep[0]);
    } else if (ret < 0) {
      epoll_err = true;
      break;
    }
    // StopProcessingEvents has been called or we have been asked to stop
    // polling. Break from the loop.
    if (data->stop_polling_.IsSignaled())
      break;

    count = epoll_wait(epoll_fd, ep, MAX_EVENTS, -1);
    // Break if epoll wait returned value less than 0 and we aren't interrupted
    // by a signal.
    if (count < 0 && errno != EINTR) {
      LOG(ERROR) << "epoll_wait returned an error." << errno;
      epoll_err = true;
      break;
    }

    for (i = 0; i < count; i++) {
      event = ep[i].events;
      // We can have cases where EPOLLIN and EPOLLHUP are both set for
      // example. Don't break if both flags are set.
      if ((event & EPOLLERR || event & EPOLLHUP) &&
             !(event & EPOLLIN)) {
        epoll_err = true;
        break;
      }

      if (event & EPOLLIN) {
        ret = wl_display_dispatch(data->display_);
        if (ret == -1) {
          LOG(ERROR) << "wl_display_dispatch failed with an error." << errno;
          epoll_err = true;
          break;
        }
      }
    }

    if (epoll_err)
      break;
  }

  data->polling_.Reset();
  data->stop_polling_.Reset();
}

}  // namespace ozonewayland
