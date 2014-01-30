// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_IPC_DISPLAY_CHANNEL_H_
#define OZONE_IMPL_IPC_DISPLAY_CHANNEL_H_

#include "base/strings/string16.h"
#include "ipc/ipc_listener.h"
#include "ozone/ui/events/window_constants.h"

namespace ozonewayland {

// OzoneDisplayChannel is responsible for listening to any messages sent by it's
// host counterpart in BrowserProcess. There will be always only one
// OzoneDisplayChannel per browser instance.

class OzoneDisplayChannel : public IPC::Listener {
 public:
  OzoneDisplayChannel();
  virtual ~OzoneDisplayChannel();

  // IPC::Listener implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  void OnEstablishChannel();
  void Register();
  void OnWidgetStateChanged(unsigned handleid,
                            WidgetState state,
                            unsigned width,
                            unsigned height);
  void OnWidgetTitleChanged(unsigned widget, base::string16 title);
  void OnWidgetAttributesChanged(unsigned widget,
                                 unsigned parent,
                                 unsigned x,
                                 unsigned y,
                                 WidgetType type);

 private:
  DISALLOW_COPY_AND_ASSIGN(OzoneDisplayChannel);
};

}  // namespace ozonewayland

#endif  // OZONE_IMPL_IPC_DISPLAY_CHANNEL_H_
