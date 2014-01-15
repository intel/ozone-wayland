// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_DISPATCHER_DELEGATE_H_
#define OZONE_WAYLAND_DISPATCHER_DELEGATE_H_

#include "base/message_loop/message_loop.h"

namespace ozonewayland {

class WindowChangeObserver;

// WaylandDispatcherDelegate can be used to customize the behaviour of
// WaylandDispatcher. WaylandDispatcherDelegate is responsible for dispatching
// events for both the single-processed and multi-processed mode

class WaylandDispatcherDelegate {
  public:
  WaylandDispatcherDelegate();
  virtual ~WaylandDispatcherDelegate();

  virtual void MotionNotify(float x, float y) = 0;
  virtual void ButtonNotify(unsigned handle,
                            int state,
                            int flags,
                            float x,
                            float y) = 0;
  virtual void AxisNotify(float x, float y, int xoffset, int yoffset) = 0;
  virtual void PointerEnter(unsigned handle, float x, float y) = 0;
  virtual void PointerLeave(unsigned handle, float x, float y) = 0;
  virtual void KeyNotify(unsigned type, unsigned code, unsigned modifiers) = 0;

  virtual void SetActive(bool active);

  virtual void OutputSizeChanged(unsigned width, unsigned height);
  virtual void SetWindowChangeObserver(WindowChangeObserver* observer);

  protected:
  // Posts task to main loop of the thread on which Dispatcher was initialized.
  virtual void PostTaskOnMainLoop(const base::Closure& task);

  base::MessageLoop* loop_;
  bool ignore_task_ :1;
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_DISPATCHER_DELEGATE_H_
