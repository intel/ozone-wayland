// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_EVENTS_EVENT_FACTORY_OZONE_WAYLAND_H_
#define OZONE_UI_EVENTS_EVENT_FACTORY_OZONE_WAYLAND_H_

#include "ozone/ui/events/event_converter_ozone_wayland.h"
#include "ozone/platform/ozone_export_wayland.h"

namespace ui {

class IMEChangeObserver;
class OutputChangeObserver;
class OzoneChannelHost;
class WindowChangeObserver;
class WindowStateChangeHandler;
class IMEStateChangeHandler;

class OZONE_WAYLAND_EXPORT EventFactoryOzoneWayland {
 public:
  explicit EventFactoryOzoneWayland(OzoneChannelHost* host = NULL);
  virtual ~EventFactoryOzoneWayland();

  // Returns the static instance last set using SetInstance().
  static EventFactoryOzoneWayland* GetInstance();

  // Sets the window change observer. Ownership is retained by the caller.
  void SetWindowChangeObserver(WindowChangeObserver* observer);
  WindowChangeObserver* GetWindowChangeObserver() const;
  // Sets the output change observer. Ownership is retained by the caller.
  void SetOutputChangeObserver(OutputChangeObserver* observer);
  // Registered OutputChangeObserver. Ownership is not passed to the caller.
  OutputChangeObserver* GetOutputChangeObserver() const;

  // Sets the ime change observer. Ownership is retained by the caller.
  void SetIMEChangeObserver(IMEChangeObserver* observer);
  // Registered IMEChangeObserver. Ownership is not passed to the caller.
  IMEChangeObserver* GetIMEChangeObserver() const;

  // EventConverter used to convert native input events to |ui::Event|.
  // Ownership is not passed to the caller.
  EventConverterOzoneWayland* GetEventConverter() const;

  // Sets the window state change handler. Ownership is retained by the caller.
  void SetWindowStateChangeHandler(WindowStateChangeHandler* statehandler);
  // Registered WindowStateChangeHandler. Ownership is not passed to the caller.
  WindowStateChangeHandler* GetWindowStateChangeHandler() const;

  // Sets the IME state change handler. Ownership is retained by the caller.
  void SetIMEStateChangeHandler(IMEStateChangeHandler* handler);
  // Registered IMEStateChangeHandler. Ownership is not passed to the caller.
  IMEStateChangeHandler* GetImeStateChangeHandler() const;

 protected:
  WindowChangeObserver* observer_;
  IMEChangeObserver* ime_observer_;
  OutputChangeObserver* output_observer_;
  WindowStateChangeHandler* state_change_handler_;
  IMEStateChangeHandler* ime_state_handler_;
  OzoneChannelHost* host_;
  scoped_ptr<ui::EventConverterOzoneWayland> event_converter_;
  static EventFactoryOzoneWayland* impl_;  // not owned
};

}  // namespace ui

#endif  // OZONE_UI_EVENTS_EVENT_FACTORY_OZONE_WAYLAND_H_
