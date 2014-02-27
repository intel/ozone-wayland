// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_EVENTS_EVENT_FACTORY_OZONE_WAYLAND_H_
#define OZONE_UI_EVENTS_EVENT_FACTORY_OZONE_WAYLAND_H_

#include "ozone/ui/events/event_converter_ozone_wayland.h"
#include "ui/events/ozone/event_factory_ozone.h"

namespace ozonewayland {

class WindowChangeObserver;
class OutputChangeObserver;

class EventFactoryOzoneWayland : public ui::EventFactoryOzone {
  public:
  EventFactoryOzoneWayland();
  virtual ~EventFactoryOzoneWayland();

  // Returns the static instance last set using SetInstance().
  static EventFactoryOzoneWayland* GetInstance();

  // Sets the implementation delegate. Ownership is retained by the caller.
  static void SetInstance(EventFactoryOzoneWayland* instance);

  // Sets the window change observer. Ownership is retained by the caller.
  void SetWindowChangeObserver(WindowChangeObserver* observer);
  // Sets the output change observer. Ownership is retained by the caller.
  void SetOutputChangeObserver(OutputChangeObserver* observer);
  // Registered OutputChangeObserver. Ownership is not passed to the caller.
  OutputChangeObserver* GetOutputChangeObserver() const;
  // EventConverter used to convert native input events to |ui::Event|.
  // Ownership is not passed to the caller.
  EventConverterOzoneWayland* EventConverter() const;

  // Overridden from EventFactoryOzone:
  // This should be called from DRWH to initialize EventConverter and start
  // processing events.
  virtual void StartProcessingEvents() OVERRIDE;
  // This should be called to start processing events in GpuProcess side. The
  // expectation is that this is called from GPU process side in multi process
  // case.
  void StartProcessingRemoteEvents();

  protected:
  EventConverterOzoneWayland* event_converter_;
  WindowChangeObserver* observer_;
  OutputChangeObserver* output_observer_;
  static EventFactoryOzoneWayland* impl_;  // not owned
};

}  // namespace ozonewayland

#endif  // OZONE_UI_EVENTS_EVENT_FACTORY_OZONE_WAYLAND_H_
