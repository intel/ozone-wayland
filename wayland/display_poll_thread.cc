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
  long flags;

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
}

WaylandDisplayPollThread::~WaylandDisplayPollThread() {
  DCHECK(!polling_.IsSignaled());
  Stop();
}

void WaylandDisplayPollThread::StartProcessingEvents() {
  DCHECK(!polling_.IsSignaled());
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
}

void  WaylandDisplayPollThread::DisplayRun(WaylandDisplayPollThread* data) {
  struct epoll_event ep[MAX_EVENTS];
  int i, ret, count = 0;
  uint32_t event = 0;
  bool epoll_err = false;
  unsigned display_fd = wl_display_get_fd(data->display_);
  int epoll_fd = osEpollCreateCloExec();
  if (epoll_fd < 0 ) {
    LOG(ERROR) << "Epoll creation failed.";
    return;
  }

  ep[0].events = EPOLLIN | EPOLLOUT;
  ep[0].data.ptr = 0;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, display_fd, &ep[0]) < 0) {
    close(epoll_fd);
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
    if (wl_display_flush(data->display_) < 0) {
      if (errno != EAGAIN) {
        epoll_err = true;
      } else {
        ep[0].events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, display_fd, &ep[0]) < 0) {
          epoll_err = true;
        }
      }

      if (epoll_err)
        break;
    }

    // StopProcessingEvents has been called or we have been asked to stop
    // polling. Break from the loop.
    if (data->stop_polling_.IsSignaled())
      break;

    count = epoll_wait(epoll_fd, ep, MAX_EVENTS, -1);
    for (i = 0; i < count; i++) {
      event = ep[i].events;

      if (event & EPOLLERR || event & EPOLLHUP) {
        epoll_err = true;
        break;
      }

      if (event & EPOLLIN) {
        ret = wl_display_dispatch(data->display_);
        if (ret == -1) {
          epoll_err = true;
          break;
        }
      }

      if (event & EPOLLOUT) {
        ret = wl_display_flush(data->display_);
        if (ret == 0) {
          struct epoll_event eps;
          memset(&eps, 0, sizeof(eps));

          eps.events = EPOLLIN | EPOLLERR | EPOLLHUP;
          epoll_ctl(epoll_fd, EPOLL_CTL_MOD, display_fd, &eps);
        } else if (ret == -1 && errno != EAGAIN) {
          epoll_err = true;
          break;
        }
      }
    }

    if (epoll_err)
      break;
  }

  close(epoll_fd);
  data->polling_.Reset();
  data->stop_polling_.Reset();
}

}  // namespace ozonewayland
