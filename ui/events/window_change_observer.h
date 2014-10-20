// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_EVENTS_WINDOW_CHANGE_OBSERVER_H_
#define OZONE_UI_EVENTS_WINDOW_CHANGE_OBSERVER_H_

#include <string>

#include "ozone/platform/ozone_export_wayland.h"
namespace ui {

// A simple observer interface for all clients interested in recieving various
// window state change notifications like when the pointer enters a
// particular window.
class OZONE_WAYLAND_EXPORT WindowChangeObserver {
 public:
  // Called when the pointer enters a window and recieves a button down
  // notification.
  virtual void OnWindowFocused(unsigned windowhandle) = 0;
  // Called when the pointer enters a window.
  virtual void OnWindowEnter(unsigned windowhandle) = 0;
  // Called when the pointer leaves a window.
  virtual void OnWindowLeave(unsigned windowhandle) = 0;
  // Called when a window is closed.
  virtual void OnWindowClose(unsigned windowhandle) = 0;
  // Called when a window is resized by server.
  virtual void OnWindowResized(unsigned windowhandle,
                               unsigned width,
                               unsigned height) = 0;
  // Called when a window is Restored.
  virtual void OnWindowUnminimized(unsigned windowhandle) = 0;
  // Called when a window is DeActivated.
  virtual void OnWindowDeActivated(unsigned windowhandle) = 0;
  // Called when a window is Activate.
  virtual void OnWindowActivated(unsigned windowhandle) = 0;

 protected:
  virtual ~WindowChangeObserver() {}
};

}  // namespace ui

#endif  // OZONE_UI_EVENTS_WINDOW_CHANGE_OBSERVER_H_
