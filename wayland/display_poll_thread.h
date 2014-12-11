// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_DISPLAY_POLL_THREAD_H_
#define OZONE_WAYLAND_DISPLAY_POLL_THREAD_H_

#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"

class wl_display;
namespace ozonewayland {
// This class lets you poll on a given Wayland display (passed in constructor),
// read any pending events coming from Wayland compositor and dispatch them.
// Caller should ensure that StopProcessingEvents is called before display is
// destroyed.
class WaylandDisplayPollThread : public base::Thread {
 public:
  explicit WaylandDisplayPollThread(wl_display* display);
  ~WaylandDisplayPollThread() override;

  // Starts polling on wl_display fd and read/flush requests coming from Wayland
  // compositor.
  void StartProcessingEvents();
  // Stops polling and handling of any events from Wayland compositor.
  void StopProcessingEvents();

 protected:
  void CleanUp() override;

 private:
  static void DisplayRun(WaylandDisplayPollThread* data);
  base::WaitableEvent polling_;  // Is set as long as the thread is polling.
  base::WaitableEvent stop_polling_;
  wl_display* display_;
  DISALLOW_COPY_AND_ASSIGN(WaylandDisplayPollThread);
};

}  // namespace ozonewayland

#endif
