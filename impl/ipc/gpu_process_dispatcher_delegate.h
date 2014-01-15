// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_IPC_GPU_PROCESS_DISPATCHER_DELEGATE_H_
#define OZONE_IMPL_IPC_GPU_PROCESS_DISPATCHER_DELEGATE_H_

#include "ozone/wayland/dispatcher_delegate.h"

namespace ozonewayland {

// GpuProcessDispatcherDelegate is used by Dispatcher in GpuProcess to send IPC
// messages to browser process. Delegate posts task to child thread, from here
// it is sent via an IPC channel to browser process.

class GpuProcessDispatcherDelegate : public WaylandDispatcherDelegate {
  public:
  GpuProcessDispatcherDelegate();
  virtual ~GpuProcessDispatcherDelegate();

  virtual void MotionNotify(float x, float y) OVERRIDE;
  virtual void ButtonNotify(unsigned handle,
                            ui::EventType type,
                            ui::EventFlags flags,
                            float x,
                            float y) OVERRIDE;
  virtual void AxisNotify(float x,
                          float y,
                          int xoffset,
                          int yoffset) OVERRIDE;
  virtual void PointerEnter(unsigned handle, float x, float y) OVERRIDE;
  virtual void PointerLeave(unsigned handle, float x, float y) OVERRIDE;
  virtual void KeyNotify(ui::EventType type,
                         unsigned code,
                         unsigned modifiers) OVERRIDE;

  virtual void OutputSizeChanged(unsigned width, unsigned height) OVERRIDE;

 private:
  static void SendMotionNotify(float x, float y);
  static void SendButtonNotify(unsigned handle,
                               ui::EventType type,
                               ui::EventFlags flags,
                               float x,
                               float y);
  static void SendAxisNotify(float x, float y, int xoffset, int yoffset);
  static void SendPointerEnter(unsigned handle, float x, float y);
  static void SendPointerLeave(unsigned handle, float x, float y);
  static void SendKeyNotify(ui::EventType type,
                            unsigned code,
                            unsigned modifiers);
  static void SendOutputSizeChanged(unsigned width, unsigned height);
  DISALLOW_COPY_AND_ASSIGN(GpuProcessDispatcherDelegate);
};

}  // namespace ozonewayland

#endif  // OZONE_IMPL_IPC_GPU_PROCESS_DISPATCHER_DELEGATE_H_
