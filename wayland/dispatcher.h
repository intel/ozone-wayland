// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_DISPATCHER_H_
#define OZONE_WAYLAND_DISPATCHER_H_

#include "base/threading/thread.h"

namespace ozonewayland {
class WindowChangeObserver;

// WaylandDispatcher class is used by OzoneDisplay for reading pending events
// coming from Wayland compositor and flush requests back. WaylandDispatcher is
// performed entirely in a separate IO thread and it can use polling as needed
// for the operations.

class WaylandDispatcher : public base::Thread {
 public:
  enum Task {
    Flush = 0x01,  // Handles Flush in worker thread.
    Poll = 0x02  // To poll on a display fd. Task has no effect in case
                 // a valid display fd is not passed to WaylandDispatcher.
  };

  explicit WaylandDispatcher(int fd = 0);
  virtual ~WaylandDispatcher();

  static WaylandDispatcher* GetInstance() { return instance_; }

  // Posts task to worker thread.
  void PostTask(Task type = Flush);
 private:
  static void HandleFlush();
  static void DisplayRun(WaylandDispatcher* data);
  bool active_ :1;
  int epoll_fd_;
  int display_fd_;
  static WaylandDispatcher* instance_;
  DISALLOW_COPY_AND_ASSIGN(WaylandDispatcher);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_DISPATCHER_H_
