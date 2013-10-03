// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_DISPLAY_CHANNEL_HOST_H_
#define OZONE_WAYLAND_DISPLAY_CHANNEL_HOST_H_

#include <queue>

#include "ozone/wayland/dispatcher.h"
#include "content/browser/gpu/gpu_process_host.h"

namespace IPC {
class Channel;
}

namespace ozonewayland {

// OzoneDisplayChannelHost is responsible for listening to any relevant messages
// sent from gpu process(i.e dispatcher and OzoneDisplayChannel). There will
// always be only one OzoneDisplayChannelHost per browser instance. It listens
// to these messages in IO thread.

class OzoneDisplayChannelHost : public IPC::ChannelProxy::MessageFilter {
 public:
  typedef std::queue<IPC::Message*> DeferredMessages;
  OzoneDisplayChannelHost();
  ~OzoneDisplayChannelHost();

  void EstablishChannel();
  void ChannelClosed();

  void SendWidgetState(unsigned w,
                       unsigned state,
                       unsigned width,
                       unsigned height);

  void OnChannelEstablished(unsigned router_id);
  void OnMotionNotify(float x, float y);
  void OnButtonNotify(int state, int flags, float x, float y);
  void OnAxisNotify(float x, float y, float xoffset, float yoffset);
  void OnPointerEnter(float x, float y);
  void OnPointerLeave(float x, float y);
  void OnKeyNotify(unsigned type, unsigned code, unsigned modifiers);
  void OnOutputSizeChanged(unsigned width, unsigned height);

  // IPC::ChannelProxy::MessageFilter implementation:
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
  virtual void OnFilterAdded(IPC::Channel* channel) OVERRIDE;
  virtual void OnChannelClosing() OVERRIDE;

  bool Send(IPC::Message* message);
  bool UpdateConnection(int gpu_id);

 private:
  WaylandDispatcher* dispatcher_;
  IPC::Channel* channel_;
  // Messages are not sent by host until connection is established. Host queues
  // all these messages to send after connection is established.
  DeferredMessages deferred_messages_;
  unsigned host_id_;
  unsigned router_id_;
  DISALLOW_COPY_AND_ASSIGN(OzoneDisplayChannelHost);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_DISPLAY_CHANNEL_HOST_H_
