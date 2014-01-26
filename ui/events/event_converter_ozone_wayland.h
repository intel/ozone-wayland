// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_EVENTS_EVENT_CONVERTER_OZONE_WAYLAND_H_
#define OZONE_UI_EVENTS_EVENT_CONVERTER_OZONE_WAYLAND_H_

#include "base/message_loop/message_loop.h"
#include "ui/events/event_constants.h"

namespace ozonewayland {

class WindowChangeObserver;
class OutputChangeObserver;

// In OzoneWayland, Chrome relies on Wayland protocol to recieve callback of
// any input/surface events. This class is responsible for the following:
// 1) Convert native input events to |ui::Event| and dispatching to aura.
// 2) Convert native events related to Window Activation and dispatch it to
//    WindowChangeObserver
class EventConverterOzoneWayland {
  public:
  EventConverterOzoneWayland();
  virtual ~EventConverterOzoneWayland();

  // Returns the static instance last set using SetInstance().
  static EventConverterOzoneWayland* GetInstance();

  // Sets the implementation delegate. Ownership is retained by the caller.
  static void SetInstance(EventConverterOzoneWayland* instance);

  virtual void MotionNotify(float x, float y) = 0;
  virtual void ButtonNotify(unsigned handle,
                            ui::EventType type,
                            ui::EventFlags flags,
                            float x,
                            float y) = 0;
  virtual void AxisNotify(float x, float y, int xoffset, int yoffset) = 0;
  virtual void PointerEnter(unsigned handle, float x, float y) = 0;
  virtual void PointerLeave(unsigned handle, float x, float y) = 0;
  virtual void KeyNotify(ui::EventType type,
                         unsigned code,
                         unsigned modifiers) = 0;

  virtual void OutputSizeChanged(unsigned width, unsigned height) = 0;
  virtual void CloseWidget(unsigned handle) = 0;
  // Sets the window change observer. Ownership is retained by the caller.
  virtual void SetWindowChangeObserver(WindowChangeObserver* observer);
  // Sets the output change observer. Ownership is retained by the caller.
  virtual void SetOutputChangeObserver(OutputChangeObserver* observer);

  protected:
  // Posts task to main loop of the thread on which Dispatcher was initialized.
  virtual void PostTaskOnMainLoop(const base::Closure& task);

  base::MessageLoop* loop_;
  static EventConverterOzoneWayland* impl_;  // not owned
};

}  // namespace ozonewayland

#endif  // OZONE_UI_EVENTS_EVENT_CONVERTER_OZONE_WAYLAND_H_
