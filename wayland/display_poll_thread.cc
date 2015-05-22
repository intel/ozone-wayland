// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/display_poll_thread.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <wayland-client.h>

#include "base/bind.h"
#include "ozone/wayland/display.h"

namespace ozonewayland {
const int MAX_EVENTS = 16;

WaylandDisplayPollThread::WaylandDisplayPollThread(wl_display* display)
    : base::Thread("WaylandDisplayPollThread"),
      display_(display),
      polling_(true, false),
      stop_polling_(true, false) {
  DCHECK(display_);
}

WaylandDisplayPollThread::~WaylandDisplayPollThread() {
  StopProcessingEvents();
}

void WaylandDisplayPollThread::StartProcessingEvents() {
  DCHECK(!polling_.IsSignaled());
  base::Thread::Options options;
  options.message_loop_type = base::MessageLoop::TYPE_IO;
  options.priority = base::ThreadPriority::BACKGROUND;
  StartWithOptions(options);
  task_runner()->PostTask(FROM_HERE, base::Bind(
      &WaylandDisplayPollThread::DisplayRun, this));
}

void WaylandDisplayPollThread::StopProcessingEvents() {
  if (polling_.IsSignaled())
    stop_polling_.Signal();

  Stop();
}

void WaylandDisplayPollThread::CleanUp() {
  SetThreadWasQuitProperly(true);
}

void  WaylandDisplayPollThread::DisplayRun(WaylandDisplayPollThread* data) {
  struct pollfd pollfd;
  int i, ret, count = 0;
  uint32_t event = 0;
  unsigned display_fd = wl_display_get_fd(data->display_);
  pollfd.fd = display_fd;
  pollfd.events = POLLIN | POLLERR | POLLHUP;

  // Set the signal state. This is used to query from other threads (i.e.
  // StopProcessingEvents on Main thread), if this thread is still polling.
  data->polling_.Signal();

  // Adopted from:
  // http://cgit.freedesktop.org/wayland/weston/tree/clients/window.c#n5531.
  while (1) {
    wl_display_dispatch_pending(data->display_);
    ret = wl_display_flush(data->display_);
    if (ret < 0 && errno != EAGAIN) {
      break;
    }
    // StopProcessingEvents has been called or we have been asked to stop
    // polling. Break from the loop.
    if (data->stop_polling_.IsSignaled())
      break;

    count = poll(&pollfd, 1, -1);
    if (count < 0 && errno != EINTR) {
      LOG(ERROR) << "poll returned an error." << errno;
      break;
    }

    if (count == 1) {
      event = pollfd.revents;
      // We can have cases where POLLIN and POLLHUP are both set for
      // example. Don't break if both flags are set.
      if ((event & POLLERR || event & POLLHUP) &&
             !(event & POLLIN)) {
        break;
      }

      if (event & POLLIN) {
        ret = wl_display_dispatch(data->display_);
        if (ret == -1) {
          LOG(ERROR) << "wl_display_dispatch failed with an error." << errno;
          break;
        }
      }
    }
  }

  data->polling_.Reset();
  data->stop_polling_.Reset();
}

}  // namespace ozonewayland
