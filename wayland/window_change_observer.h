// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOW_CHANGE_OBSERVER_H_
#define WINDOW_CHANGE_OBSERVER_H_

namespace ozonewayland {

// A simple observer interface for all clients interested in recieving various
// window state change notifications like when the pointer enters a
// particular window.
class WindowChangeObserver {
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

 protected:
  virtual ~WindowChangeObserver() {}
};

}  // namespace ozonewayland

#endif  // WINDOW_CHANGE_OBSERVER_H_
