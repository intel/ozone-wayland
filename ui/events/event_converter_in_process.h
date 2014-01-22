// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_EVENTS_EVENT_CONVERTER_IN_PROCESS_H_
#define OZONE_UI_EVENTS_EVENT_CONVERTER_IN_PROCESS_H_

#include "base/memory/scoped_ptr.h"
#include "ozone/ui/events/event_converter_ozone_wayland.h"
#include "ui/events/event.h"

namespace ozonewayland {

class EventConverterInProcess : public EventConverterOzoneWayland {
  public:
  EventConverterInProcess();
  virtual ~EventConverterInProcess();

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
  virtual void CloseWidget(unsigned handle) OVERRIDE;

  virtual void SetWindowChangeObserver(WindowChangeObserver* observer) OVERRIDE;

 private:
  static void NotifyPointerEnter(EventConverterInProcess* data,
                                 unsigned handle);
  static void NotifyPointerLeave(EventConverterInProcess* data,
                                 unsigned handle);
  static void NotifyButtonPress(EventConverterInProcess* data,
                                unsigned handle);
  static void NotifyCloseWidget(EventConverterInProcess* data,
                                unsigned handle);
  static void DispatchEventHelper(scoped_ptr<ui::Event> key);
  WindowChangeObserver* observer_;
  DISALLOW_COPY_AND_ASSIGN(EventConverterInProcess);
};

}  // namespace ozonewayland

#endif  // OZONE_UI_EVENTS_EVENT_CONVERTER_IN_PROCESS_H_
