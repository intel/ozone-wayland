// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/content/remote_state_change_handler.h"

#include "base/bind.h"
#include "content/browser/gpu/browser_gpu_channel_host_factory.h"
#include "content/public/browser/browser_child_process_host_iterator.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/process_type.h"
#include "ozone/content/messages.h"

namespace content {

// This should be same as defined in display_channel.
const int CHANNEL_ROUTE_ID = -0x1;

RemoteStateChangeHandler::RemoteStateChangeHandler()
    : iterator_(NULL) {
  WindowStateChangeHandler::SetInstance(this);
  IMEStateChangeHandler::SetInstance(this);
  BrowserGpuChannelHostFactory* host_factory =
      BrowserGpuChannelHostFactory::instance();
  DCHECK(host_factory);
  // This is a synchronous call and blocks current thread till Channel is
  // setup.
  BrowserGpuChannelHostFactory::instance()->EstablishGpuChannelSync(
      CAUSE_FOR_GPU_LAUNCH_BROWSER_STARTUP);

  BrowserThread::PostTask(BrowserThread::IO, FROM_HERE,
      base::Bind(&RemoteStateChangeHandler::EstablishChannel,
          base::Unretained(this)));
}

RemoteStateChangeHandler::~RemoteStateChangeHandler() {
}

void RemoteStateChangeHandler::SetWidgetState(unsigned w,
                                             ui::WidgetState state,
                                             unsigned width,
                                             unsigned height) {
  if (!BrowserThread::CurrentlyOn(BrowserThread::IO)) {
    BrowserThread::PostTask(BrowserThread::IO, FROM_HERE,
        base::Bind(&RemoteStateChangeHandler::SetWidgetState,
            base::Unretained(this), w, state, width, height));
    return;
  }

  Send(new WaylandWindow_State(CHANNEL_ROUTE_ID, w, state, width, height));
}

void RemoteStateChangeHandler::SetWidgetTitle(unsigned w,
                                             const base::string16& title) {
  if (!BrowserThread::CurrentlyOn(BrowserThread::IO)) {
    BrowserThread::PostTask(BrowserThread::IO, FROM_HERE,
        base::Bind(&RemoteStateChangeHandler::SetWidgetTitle,
            base::Unretained(this), w, title));
    return;
  }

  Send(new WaylandWindow_Title(CHANNEL_ROUTE_ID, w, title));
}

void RemoteStateChangeHandler::SetWidgetAttributes(unsigned widget,
                                                  unsigned parent,
                                                  unsigned x,
                                                  unsigned y,
                                                  ui::WidgetType type) {
  if (!BrowserThread::CurrentlyOn(BrowserThread::IO)) {
    BrowserThread::PostTask(BrowserThread::IO, FROM_HERE,
        base::Bind(&RemoteStateChangeHandler::SetWidgetAttributes,
            base::Unretained(this), widget, parent, x, y, type));
    return;
  }

  Send(new WaylandWindow_Attributes(CHANNEL_ROUTE_ID,
                                    widget,
                                    parent,
                                    x,
                                    y,
                                    type));
}

void RemoteStateChangeHandler::ResetIme() {
  if (!BrowserThread::CurrentlyOn(BrowserThread::IO)) {
    BrowserThread::PostTask(BrowserThread::IO, FROM_HERE,
        base::Bind(&RemoteStateChangeHandler::ResetIme,
            base::Unretained(this)));
    return;
  }

  Send(new WaylandWindow_ImeReset(CHANNEL_ROUTE_ID));
}

void RemoteStateChangeHandler::ImeCaretBoundsChanged(gfx::Rect rect) {
  if (!BrowserThread::CurrentlyOn(BrowserThread::IO)) {
    BrowserThread::PostTask(BrowserThread::IO, FROM_HERE,
        base::Bind(&RemoteStateChangeHandler::ImeCaretBoundsChanged,
            base::Unretained(this), rect));
    return;
  }

  Send(new WaylandWindow_ImeCaretBoundsChanged(CHANNEL_ROUTE_ID, rect));
}

bool RemoteStateChangeHandler::Send(IPC::Message* message) {
  // The GPU process never sends synchronous IPC, so clear the unblock flag.
  // This ensures the message is treated as a synchronous one and helps preserve
  // order. Check set_unblock in ipc_messages.h for explanation.
  message->set_unblock(true);
  return iterator_->Send(message);
}

void RemoteStateChangeHandler::EstablishChannel() {
  DCHECK(!iterator_);
  iterator_ = new BrowserChildProcessHostIterator(PROCESS_TYPE_GPU);
  DCHECK(!iterator_->Done());
}

}  // namespace content
