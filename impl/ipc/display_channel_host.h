// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_IPC_DISPLAY_CHANNEL_HOST_H_
#define OZONE_IMPL_IPC_DISPLAY_CHANNEL_HOST_H_

#include <queue>

#include "content/browser/gpu/gpu_process_host.h"
#include "ui/events/event_constants.h"

namespace IPC {
class Channel;
}

namespace ozonewayland {

class WaylandDispatcher;

// OzoneDisplayChannelHost is responsible for listening to any relevant messages
// sent from gpu process(i.e dispatcher and OzoneDisplayChannel). There will
// always be only one OzoneDisplayChannelHost per browser instance. It listens
// to these messages in IO thread.

class OzoneDisplayChannelHost : public IPC::ChannelProxy::MessageFilter {
 public:
  typedef std::queue<IPC::Message*> DeferredMessages;
  OzoneDisplayChannelHost();

  void EstablishChannel();

  void SendWidgetState(unsigned w,
                       unsigned state,
                       unsigned width,
                       unsigned height);
  void SendWidgetTitle(unsigned w, const base::string16& title);
  void SendWidgetAttributes(unsigned widget,
                            unsigned parent,
                            unsigned x,
                            unsigned y,
                            unsigned type);
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

  // IPC::ChannelProxy::MessageFilter implementation:
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
  virtual void OnFilterAdded(IPC::Channel* channel) OVERRIDE;
  virtual void OnChannelClosing() OVERRIDE;

  bool Send(IPC::Message* message);

 private:
  virtual ~OzoneDisplayChannelHost();
  void OnChannelEstablished();
  void UpdateConnection();
  WaylandDispatcher* dispatcher_;
  IPC::Channel* channel_;
  // Messages are not sent by host until connection is established. Host queues
  // all these messages to send after connection is established.
  DeferredMessages deferred_messages_;
  DISALLOW_COPY_AND_ASSIGN(OzoneDisplayChannelHost);
};

}  // namespace ozonewayland

#endif  // OZONE_IMPL_IPC_DISPLAY_CHANNEL_HOST_H_
