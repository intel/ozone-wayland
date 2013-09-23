// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_DISPLAY_CHANNEL_HOST_H_
#define OZONE_WAYLAND_DISPLAY_CHANNEL_HOST_H_

#include "ozone/wayland/dispatcher.h"
#include "content/public/browser/browser_message_filter.h"
#include "content/browser/gpu/gpu_process_host.h"

namespace ozonewayland {

// OzoneDisplayChannelHost is responsible for listening to any relevant messages
// sent from gpu process(i.e dispatcher and OzoneDisplayChannel). There will
// always be only one OzoneDisplayChannelHost per browser instance. It listens
// to these messages in IO thread.

class OzoneDisplayChannelHost : public content::BrowserMessageFilter {
 public:
  typedef std::queue<IPC::Message*> DeferredMessages;
  OzoneDisplayChannelHost();
  ~OzoneDisplayChannelHost();

  void EstablishChannel(unsigned process_id);
  void ChannelClosed(unsigned process_id);

  void SendWidgetState(unsigned w, unsigned state);

  void OnChannelEstablished(unsigned router_id);
  void OnMotionNotify(float x, float y);
  void OnButtonNotify(int state, int flags, float x, float y);
  void OnAxisNotify(float x, float y, float xoffset, float yoffset);
  void OnPointerEnter(float x, float y);
  void OnPointerLeave(float x, float y);
  void OnKeyNotify(unsigned type, unsigned code, unsigned modifiers);
  void OnOutputSizeChanged(unsigned width, unsigned height);

  // IPC::Listener implementation:
  virtual bool OnMessageReceived(const IPC::Message& message,
                                 bool* message_was_ok) OVERRIDE;
  bool UpdateConnection(int process_id);

 private:
  WaylandDispatcher* dispatcher_;
  // Messages are not sent by host until connection is established. Host queues
  // all these messages to send after connection is established.
  DeferredMessages deferred_messages_;
  unsigned process_id_;
  unsigned host_id_;
  unsigned router_id_;
  DISALLOW_COPY_AND_ASSIGN(OzoneDisplayChannelHost);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_DISPLAY_CHANNEL_HOST_H_
