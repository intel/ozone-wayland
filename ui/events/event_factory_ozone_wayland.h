// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_EVENTS_EVENT_FACTORY_OZONE_WAYLAND_H_
#define OZONE_UI_EVENTS_EVENT_FACTORY_OZONE_WAYLAND_H_

#include "ozone/ui/events/event_converter_ozone_wayland.h"
#include "ozone/platform/ozone_export_wayland.h"
#include "ui/ozone/public/event_factory_ozone.h"

namespace ui {

class WindowChangeObserver;
class OutputChangeObserver;

class OZONE_WAYLAND_EXPORT EventFactoryOzoneWayland : public EventFactoryOzone {
 public:
  EventFactoryOzoneWayland();
  virtual ~EventFactoryOzoneWayland();

  // Returns the static instance last set using SetInstance().
  static EventFactoryOzoneWayland* GetInstance();

  // Sets the implementation delegate. Ownership is retained by the caller.
  static void SetInstance(EventFactoryOzoneWayland* instance);

  // Sets the window change observer. Ownership is retained by the caller.
  void SetWindowChangeObserver(WindowChangeObserver* observer);
  WindowChangeObserver* GetWindowChangeObserver() const;
  // Sets the output change observer. Ownership is retained by the caller.
  void SetOutputChangeObserver(OutputChangeObserver* observer);
  // Registered OutputChangeObserver. Ownership is not passed to the caller.
  OutputChangeObserver* GetOutputChangeObserver() const;
  void SetEventConverterOzoneWayland(EventConverterOzoneWayland* converter);
  // EventConverter used to convert native input events to |ui::Event|.
  // Ownership is not passed to the caller.
  EventConverterOzoneWayland* EventConverter() const;

 protected:
  EventConverterOzoneWayland* event_converter_;
  WindowChangeObserver* observer_;
  OutputChangeObserver* output_observer_;
  static EventFactoryOzoneWayland* impl_;  // not owned
};

}  // namespace ui

#endif  // OZONE_UI_EVENTS_EVENT_FACTORY_OZONE_WAYLAND_H_
