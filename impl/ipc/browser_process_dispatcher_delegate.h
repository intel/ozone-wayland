// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_IPC_BROWSER_PROCESS_DISPATCHER_DELEGATE_H_
#define OZONE_IMPL_IPC_BROWSER_PROCESS_DISPATCHER_DELEGATE_H_

#include "base/memory/scoped_ptr.h"
#include "ozone/wayland/dispatcher_delegate.h"
#include "ui/events/event.h"

namespace ozonewayland {

// BrowserProcessDispatcherDelegate is used by dispatcher to post events to
// message loop of main thread in the browserProcess. Delegate creates
// appropriate event and posts it to message loop of the main thread. The event
// is then dispatched to MessagePumpOzone.

class BrowserProcessDispatcherDelegate : public WaylandDispatcherDelegate {
  public:
  BrowserProcessDispatcherDelegate();
  virtual ~BrowserProcessDispatcherDelegate();

  virtual void MotionNotify(float x, float y) OVERRIDE;
  virtual void ButtonNotify(unsigned handle,
                            int state,
                            int flags,
                            float x,
                            float y) OVERRIDE;
  virtual void AxisNotify(float x,
                          float y,
                          float xoffset,
                          float yoffset) OVERRIDE;
  virtual void PointerEnter(unsigned handle, float x, float y) OVERRIDE;
  virtual void PointerLeave(unsigned handle, float x, float y) OVERRIDE;
  virtual void KeyNotify(unsigned type,
                         unsigned code,
                         unsigned modifiers) OVERRIDE;

  virtual void SetWindowChangeObserver(WindowChangeObserver* observer) OVERRIDE;

 private:
  static void NotifyPointerEnter(BrowserProcessDispatcherDelegate* data,
                                 unsigned handle);
  static void NotifyPointerLeave(BrowserProcessDispatcherDelegate* data,
                                 unsigned handle);
  static void NotifyButtonPress(BrowserProcessDispatcherDelegate* data,
                                unsigned handle);
  static void DispatchEventHelper(scoped_ptr<ui::Event> key);
  WindowChangeObserver* observer_;
  DISALLOW_COPY_AND_ASSIGN(BrowserProcessDispatcherDelegate);
};

}  // namespace ozonewayland

#endif  // OZONE_IMPL_IPC_BROWSER_PROCESS_DISPATCHER_DELEGATE_H_
