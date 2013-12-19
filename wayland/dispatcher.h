// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_DISPATCHER_H_
#define OZONE_WAYLAND_DISPATCHER_H_

#include "base/memory/scoped_ptr.h"
#include "base/threading/thread.h"
#include "ozone/wayland/display.h"
#include "ozone/wayland/window_change_observer.h"
#include "ui/events/event.h"

namespace ozonewayland {
class OzoneDisplay;

// WaylandDispatcher class is used by OzoneDisplay for reading pending events
// coming from Wayland compositor and flush requests back. WaylandDispatcher is
// performed entirely in a separate IO thread and it can use polling as needed
// for the operations.

// WaylandDispatcher is also used by input classes to send related events.
// In case of single process:
// Dispatcher creates appropriate event and posts it to message loop of
// mainthread. The event is than dispatched to MessagePumpOzone.
// In case of MultiProcess:
// Dispatcher posts task to childthread, from here it is sent via IPC channel to
// Browser.

class WaylandDispatcher : public base::Thread {
 public:
  enum Task {
    Flush = 0x01,  // Handles Flush in worker thread.
    Poll = 0x02  // To poll on a display fd. Task has no effect in case
                 // a valid display fd is not passed to WaylandDispatcher.
  };

  static WaylandDispatcher* GetInstance() { return instance_; }
  void MotionNotify(float x, float y);
  void ButtonNotify(unsigned handle, int state, int flags, float x, float y);
  void AxisNotify(float x, float y, float xoffset, float yoffset);
  void PointerEnter(unsigned handle, float x, float y);
  void PointerLeave(unsigned handle, float x, float y);
  void KeyNotify(unsigned type, unsigned code, unsigned modifiers);
  void OutputSizeChanged(unsigned width, unsigned height);

  // Posts task to worker thread.
  void PostTask(Task type = Flush);
  // Dispatches event to MessagePumpOzone.
  void DispatchEvent(scoped_ptr<ui::Event> event);
  // Posts task to main loop of the thread on which Dispatcher was initialized.
  void PostTaskOnMainLoop(const tracked_objects::Location& from_here,
                          const base::Closure& task);

  void SetWindowChangeObserver(WindowChangeObserver* observer) {
    observer_ = observer;
  }

 private:
  explicit WaylandDispatcher(int fd = 0);
  virtual ~WaylandDispatcher();
  static void HandleFlush();
  static void DisplayRun(WaylandDispatcher* data);
  static void NotifyPointerEnter(WaylandDispatcher* data, unsigned handle);
  static void NotifyPointerLeave(WaylandDispatcher* data, unsigned handle);
  static void NotifyButtonPress(WaylandDispatcher* data, unsigned handle);
  static void DispatchEventHelper(scoped_ptr<ui::Event> key);
  static void SendMotionNotify(float x, float y);
  static void SendButtonNotify(unsigned handle,
                               int state,
                               int flags,
                               float x,
                               float y);
  static void SendAxisNotify(float x, float y, float xoffset, float yoffset);
  static void SendPointerEnter(unsigned handle, float x, float y);
  static void SendPointerLeave(unsigned handle, float x, float y);
  static void SendKeyNotify(unsigned type, unsigned code, unsigned modifiers);
  static void SendOutputSizeChanged(unsigned width, unsigned height);
  void MessageLoopDestroyed();
  base::MessageLoop* loop_;
  bool ignore_task_ :1;
  bool running :1;
  int epoll_fd_;
  int display_fd_;
  WindowChangeObserver* observer_;
  static WaylandDispatcher* instance_;
  friend class OzoneDisplay;
  DISALLOW_COPY_AND_ASSIGN(WaylandDispatcher);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_DISPATCHER_H_
