// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_DISPLAY_CHANNEL_H_
#define OZONE_WAYLAND_DISPLAY_CHANNEL_H_

#include "ozone/wayland/dispatcher.h"
#include "ipc/ipc_listener.h"

namespace ozonewayland {

// OzoneDisplayChannel is responsible for listening to any messages sent by it's
// host counterpart in BrowserProcess. There will be always only one
// OzoneDisplayChannel per browser instance.

class OzoneDisplayChannel : public IPC::Listener
{
 public:
  OzoneDisplayChannel();
  ~OzoneDisplayChannel();

  // IPC::Listener implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  void OnEstablishChannel();
  void Register();
  void OnWidgetStateChanged(unsigned handleid,
                            unsigned state,
                            unsigned width,
                            unsigned height);
  void OnWidgetTitleChanged(unsigned widget, string16 title);

 private:
  bool mapped_ :1;
  DISALLOW_COPY_AND_ASSIGN(OzoneDisplayChannel);
};

}  // namespace ui

#endif  // OZONE_WAYLAND_DISPLAY_CHANNEL_H_
