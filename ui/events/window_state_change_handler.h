// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_EVENTS_WINDOW_STATE_CHANGE_HANDLER_H_
#define OZONE_UI_EVENTS_WINDOW_STATE_CHANGE_HANDLER_H_

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "ozone/ui/events/window_constants.h"

namespace IPC {
class Channel;
}

namespace ozonewayland {

// A simple interface for passing Window state change notifications coming from
// Aura to Wayland.
class WindowStateChangeHandler {
 public:
  WindowStateChangeHandler();
  virtual ~WindowStateChangeHandler();

  // Returns the static instance last set using SetInstance().
  static WindowStateChangeHandler* GetInstance();

  // Sets the implementation delegate. Ownership is retained by the caller.
  static void SetInstance(WindowStateChangeHandler* instance);

  // Called when AcceleratedWidget widget state has changed.
  virtual void SetWidgetState(unsigned widget,
                              WidgetState state,
                              unsigned width = 0,
                              unsigned height = 0) = 0;

  // Called when AcceleratedWidget widget title has changed.
  virtual void SetWidgetTitle(unsigned widget, const base::string16& title) = 0;

  // This is called when we want to create an AcceleratedWidget widget.
  virtual void SetWidgetAttributes(unsigned widget,
                                   unsigned parent,
                                   unsigned x,
                                   unsigned y,
                                   WidgetType type) = 0;

 private:
  static WindowStateChangeHandler* impl_;  // not owned
};

}  // namespace ozonewayland

#endif  // OZONE_UI_EVENTS_WINDOW_STATE_CHANGE_HANDLER_H_
