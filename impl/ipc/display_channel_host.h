// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_IPC_DISPLAY_CHANNEL_HOST_H_
#define OZONE_IMPL_IPC_DISPLAY_CHANNEL_HOST_H_

#include "content/browser/gpu/gpu_process_host.h"
#include "content/public/browser/browser_child_process_observer.h"
#include "content/public/browser/child_process_data.h"
#include "ui/events/event_constants.h"

namespace ui {
class EventConverterOzoneWayland;
class RemoteStateChangeHandler;
}

namespace content {

// OzoneDisplayChannelHost is responsible for listening to any relevant messages
// sent from gpu process(i.e dispatcher and OzoneDisplayChannel). There will
// always be only one OzoneDisplayChannelHost per browser instance. It listens
// to these messages in IO thread.

class OzoneDisplayChannelHost : public content::BrowserChildProcessObserver {
 public:
  OzoneDisplayChannelHost();
  virtual ~OzoneDisplayChannelHost();

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
  void OnWindowResized(unsigned handle,
                       unsigned width,
                       unsigned height);

  // Implement |BrowserChildProcessObserver|.
  virtual void BrowserChildProcessHostConnected(
    const content::ChildProcessData& data) OVERRIDE;
  virtual void BrowserChildProcessHostDisconnected(
      const content::ChildProcessData& data) OVERRIDE;
  virtual void BrowserChildProcessCrashed(
      const content::ChildProcessData& data) OVERRIDE;

 private:
  void OnMessageReceived(const IPC::Message& message);
  void EstablishChannel();
  void UpdateConnection();
  ui::EventConverterOzoneWayland* dispatcher_;
  ui::RemoteStateChangeHandler* state_handler_;
  DISALLOW_COPY_AND_ASSIGN(OzoneDisplayChannelHost);
};

}  // namespace content

#endif  // OZONE_IMPL_IPC_DISPLAY_CHANNEL_HOST_H_
