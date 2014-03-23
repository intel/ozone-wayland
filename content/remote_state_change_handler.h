// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_CONTENT_REMOTE_STATE_CHANGE_HANDLER_H_
#define OZONE_CONTENT_REMOTE_STATE_CHANGE_HANDLER_H_

#include "ozone/ui/events/ime_state_change_handler.h"
#include "ozone/ui/events/window_state_change_handler.h"

namespace IPC {
class Message;
}

namespace content {
class BrowserChildProcessHostIterator;
// RemoteStateChangeHandler implements WindowStateChangeHandler and
// IMEStateChangeHandler. It is responsible for sending any Ime/Window state
// change events from Browser to GPU process (i.e IPC).

class RemoteStateChangeHandler : public ui::WindowStateChangeHandler,
                                 public ui::IMEStateChangeHandler {
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
  virtual void SetWidgetCursor(int cursor_type) OVERRIDE;
  virtual void SetWidgetAttributes(unsigned widget,
                                   unsigned parent,
                                   unsigned x,
                                   unsigned y,
                                   ui::WidgetType type) OVERRIDE;
  virtual void ResetIme() OVERRIDE;
  virtual void ImeCaretBoundsChanged(gfx::Rect rect) OVERRIDE;
  virtual void ShowInputPanel() OVERRIDE;
  virtual void HideInputPanel() OVERRIDE;

 private:
  bool Send(IPC::Message* message);
  void EstablishChannel();

  content::BrowserChildProcessHostIterator* iterator_;
  DISALLOW_COPY_AND_ASSIGN(RemoteStateChangeHandler);
};

}  // namespace content

#endif  // OZONE_CONTENT_EVENTS_REMOTE_STATE_CHANGE_HANDLER_H_
