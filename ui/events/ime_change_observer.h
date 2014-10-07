// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_EVENTS_IME_CHANGE_OBSERVER_H_
#define OZONE_UI_EVENTS_IME_CHANGE_OBSERVER_H_

#include <string>

#include "ozone/platform/ozone_export_wayland.h"
namespace ui {

// A simple observer interface for all clients interested in recieving various
// window state change notifications like when the pointer enters a
// particular window.
class OZONE_WAYLAND_EXPORT IMEChangeObserver {
 public:
  virtual void OnPreeditChanged(unsigned handle,
                                const std::string& text,
                                const std::string& commit) = 0;
  virtual void OnCommit(unsigned handle, const std::string& text) = 0;

 protected:
  virtual ~IMEChangeObserver() {}
};

}  // namespace ui

#endif  // OZONE_UI_EVENTS_IME_CHANGE_OBSERVER_H_
