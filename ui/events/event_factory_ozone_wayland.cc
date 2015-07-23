// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/events/event_factory_ozone_wayland.h"

#include "base/basictypes.h"
#include "base/logging.h"

namespace ui {

// static
EventFactoryOzoneWayland* EventFactoryOzoneWayland::impl_ = NULL;

EventFactoryOzoneWayland::EventFactoryOzoneWayland()
    : ime_observer_(NULL),
      ime_state_handler_(NULL) {
  impl_ = this;
}

EventFactoryOzoneWayland::~EventFactoryOzoneWayland() {
  impl_ = NULL;
}

EventFactoryOzoneWayland* EventFactoryOzoneWayland::GetInstance() {
  CHECK(impl_) << "No EventFactoryOzoneWayland implementation set.";
  return impl_;
}

void EventFactoryOzoneWayland::SetIMEChangeObserver(
    IMEChangeObserver* observer) {
  ime_observer_ = observer;
}

IMEChangeObserver*
EventFactoryOzoneWayland::GetIMEChangeObserver() const {
  return ime_observer_;
}

void EventFactoryOzoneWayland::SetIMEStateChangeHandler(
    IMEStateChangeHandler* handler) {
  ime_state_handler_ = handler;
}

IMEStateChangeHandler*
EventFactoryOzoneWayland::GetImeStateChangeHandler() const {
  return ime_state_handler_;
}

}  // namespace ui
