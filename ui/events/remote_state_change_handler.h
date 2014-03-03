// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_EVENTS_REMOTE_STATE_CHANGE_HANDLER_H_
#define OZONE_UI_EVENTS_REMOTE_STATE_CHANGE_HANDLER_H_

#include "ozone/ui/events/window_state_change_handler.h"
#include "ozone/ui/ime/ime_state_change_handler.h"

namespace content {
class BrowserChildProcessHostIterator;
}

namespace IPC {
class Message;
}

namespace ui {

// RemoteStateChangeHandler implements WindowStateChangeHandler and
// IMEStateChangeHandler. It is responsible for sending any Ime/Window state
// change events from Browser to GPU process (i.e IPC).

class RemoteStateChangeHandler : public WindowStateChangeHandler,
                                 public IMEStateChangeHandler {
 public:
  RemoteStateChangeHandler();
  virtual ~RemoteStateChangeHandler();

  // WindowStateChangeHandler implementation:
  virtual void SetWidgetState(unsigned widget,
                              ui::WidgetState state,
                              unsigned width = 0,
                              unsigned height = 0) OVERRIDE;
  virtual void SetWidgetTitle(unsigned w,
                              const base::string16& title) OVERRIDE;
  virtual void SetWidgetAttributes(unsigned widget,
                                   unsigned parent,
                                   unsigned x,
                                   unsigned y,
                                   ui::WidgetType type) OVERRIDE;
  virtual void ResetIme() OVERRIDE;
  virtual void ImeCaretBoundsChanged(gfx::Rect rect) OVERRIDE;

 private:
  bool Send(IPC::Message* message);
  void EstablishChannel();

  content::BrowserChildProcessHostIterator* iterator_;
  DISALLOW_COPY_AND_ASSIGN(RemoteStateChangeHandler);
};

}  // namespace ui

#endif  // OZONE_UI_EVENTS_REMOTE_STATE_CHANGE_HANDLER_H_
