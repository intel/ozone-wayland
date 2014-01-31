// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_IPC_DISPLAY_CHANNEL_HOST_H_
#define OZONE_IMPL_IPC_DISPLAY_CHANNEL_HOST_H_

#include <queue>

#include "content/browser/gpu/gpu_process_host.h"
#include "content/public/browser/browser_child_process_observer.h"
#include "content/public/browser/child_process_data.h"
#include "ozone/ui/events/window_state_change_handler.h"
#include "ui/events/event_constants.h"

namespace IPC {
class Channel;
}

namespace ozonewayland {

class EventConverterOzoneWayland;

// OzoneDisplayChannelHost is responsible for listening to any relevant messages
// sent from gpu process(i.e dispatcher and OzoneDisplayChannel). There will
// always be only one OzoneDisplayChannelHost per browser instance. It listens
// to these messages in IO thread.

class OzoneDisplayChannelHost : public IPC::ChannelProxy::MessageFilter,
                                public content::BrowserChildProcessObserver,
                                public WindowStateChangeHandler {
 public:
  typedef std::queue<IPC::Message*> DeferredMessages;
  OzoneDisplayChannelHost();

  // WindowStateChangeHandler implementation:
  virtual void SetWidgetState(unsigned widget,
                              WidgetState state,
                              unsigned width = 0,
                              unsigned height = 0) OVERRIDE;
  virtual void SetWidgetTitle(unsigned w,
                              const base::string16& title) OVERRIDE;
  virtual void SetWidgetAttributes(unsigned widget,
                                   unsigned parent,
                                   unsigned x,
                                   unsigned y,
                                   WidgetType type) OVERRIDE;
  void OnMotionNotify(float x, float y);
  void OnButtonNotify(unsigned handle,
                      ui::EventType type,
                      ui::EventFlags flags,
                      float x,
                      float y);
  void OnAxisNotify(float x, float y, int xoffset, int yoffset);
  void OnPointerEnter(unsigned handle, float x, float y);
  void OnPointerLeave(unsigned handle, float x, float y);
  void OnKeyNotify(ui::EventType type, unsigned code, unsigned modifiers);
  void OnOutputSizeChanged(unsigned width, unsigned height);
  void OnCloseWidget(unsigned handle);

  // IPC::ChannelProxy::MessageFilter implementation:
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
  virtual void OnFilterAdded(IPC::Channel* channel) OVERRIDE;
  virtual void OnChannelClosing() OVERRIDE;

  // Implement |BrowserChildProcessObserver|.
  virtual void BrowserChildProcessHostConnected(
    const content::ChildProcessData& data) OVERRIDE;

  bool Send(IPC::Message* message);

 private:
  virtual ~OzoneDisplayChannelHost();
  void EstablishChannel();
  void OnChannelEstablished();
  void UpdateConnection();
  EventConverterOzoneWayland* dispatcher_;
  IPC::Channel* channel_;
  // Messages are not sent by host until connection is established. Host queues
  // all these messages to send after connection is established.
  DeferredMessages deferred_messages_;
  DISALLOW_COPY_AND_ASSIGN(OzoneDisplayChannelHost);
};

}  // namespace ozonewayland

#endif  // OZONE_IMPL_IPC_DISPLAY_CHANNEL_HOST_H_
