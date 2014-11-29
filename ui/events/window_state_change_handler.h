// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_EVENTS_WINDOW_STATE_CHANGE_HANDLER_H_
#define OZONE_UI_EVENTS_WINDOW_STATE_CHANGE_HANDLER_H_

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "ozone/platform/ozone_export_wayland.h"
#include "ozone/ui/events/window_constants.h"

namespace ui {

// A simple interface for passing Window state change notifications coming from
// Aura to Wayland.
class OZONE_WAYLAND_EXPORT WindowStateChangeHandler {
 public:
  virtual ~WindowStateChangeHandler() { }

  // Called when AcceleratedWidget widget state has changed.
  virtual void SetWidgetState(unsigned widget,
                              ui::WidgetState state) = 0;

  // Called when AcceleratedWidget widget title has changed.
  virtual void SetWidgetTitle(unsigned widget, const base::string16& title) = 0;
  // Called when Cursor has changed and the image needs to be updated.
  virtual void SetWidgetCursor(int cursor_type) = 0;

  // This is called when we want to create an AcceleratedWidget widget.
  virtual void CreateWidget(unsigned handle,
                            unsigned parent,
                            unsigned x,
                            unsigned y,
                            ui::WidgetType type) = 0;
};

}  // namespace ui

#endif  // OZONE_UI_EVENTS_WINDOW_STATE_CHANGE_HANDLER_H_
