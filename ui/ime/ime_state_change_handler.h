// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_IME_IME_STATE_CHANGE_HANDLER_H_
#define OZONE_UI_IME_IME_STATE_CHANGE_HANDLER_H_

#include "base/basictypes.h"
#include "ui/events/events_export.h"
#include "ui/gfx/rect.h"

namespace ui {

// A simple interface for passing IME state change notifications coming from
// Aura to Wayland.
class EVENTS_EXPORT IMEStateChangeHandler {
 public:
  IMEStateChangeHandler();
  virtual ~IMEStateChangeHandler();

  // Returns the static instance last set using SetInstance().
  static IMEStateChangeHandler* GetInstance();

  // Sets the implementation delegate. Ownership is retained by the caller.
  static void SetInstance(IMEStateChangeHandler* instance);

  // This is called with IMEContext needs to be reset.
  virtual void ResetIme() = 0;

  // Notifies the context that the caret bounds have changed.  |rect| is
  // relative to screen coordinates.
  virtual void ImeCaretBoundsChanged(gfx::Rect rect) = 0;

 private:
  static IMEStateChangeHandler* impl_;  // not owned
};

}  // namespace ui

#endif  // OZONE_UI_IME_IME_STATE_CHANGE_HANDLER_H_
