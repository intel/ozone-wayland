// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_EVENTS_OUTPUT_CHANGE_OBSERVER_H_
#define OZONE_UI_EVENTS_OUTPUT_CHANGE_OBSERVER_H_

namespace ozonewayland {

// A simple observer interface for all clients interested in receiving various
// output change notifications like size changes, when a new output is added,
// etc.
class OutputChangeObserver {
 public:
  virtual ~OutputChangeObserver() {}
  // Called when the current output size has changed.
  virtual void OnOutputSizeChanged(unsigned width, unsigned height) = 0;
};

}  // namespace ozonewayland

#endif  // OZONE_UI_EVENTS_OUPUT_CHANGE_OBSERVER_H_
