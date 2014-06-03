// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_CONTENT_OZONE_CHANNEL_HOST_H_
#define OZONE_CONTENT_OZONE_CHANNEL_HOST_H_

#include <string>

#include "content/browser/gpu/gpu_process_host.h"
#include "content/public/browser/browser_child_process_observer.h"
#include "content/public/browser/child_process_data.h"
#include "ui/events/event_constants.h"

namespace ui {
class EventConverterOzoneWayland;
}

namespace content {
class RemoteStateChangeHandler;
// OzoneChannelHost is responsible for listening to any relevant messages
// sent from gpu process(i.e dispatcher and OzoneDisplayChannel). There will
// always be only one OzoneChannelHost per browser instance. It listens
// to these messages in IO thread.

class OzoneChannelHost : public BrowserChildProcessObserver {
 public:
  OzoneChannelHost();
  virtual ~OzoneChannelHost();

  void OnMotionNotify(float x, float y);
  void OnButtonNotify(unsigned handle,
                      ui::EventType type,
                      ui::EventFlags flags,
                      float x,
                      float y);
  void OnTouchNotify(ui::EventType type,
                     float x,
                     float y,
                     int32_t touch_id,
                     uint32_t time_stamp);
  void OnAxisNotify(float x, float y, int xoffset, int yoffset);
  void OnPointerEnter(unsigned handle, float x, float y);
  void OnPointerLeave(unsigned handle, float x, float y);
  void OnKeyNotify(ui::EventType type, unsigned code, unsigned modifiers);
  void OnOutputSizeChanged(unsigned width, unsigned height);
  void OnCloseWidget(unsigned handle);
  void OnWindowResized(unsigned handle,
                       unsigned width,
                       unsigned height);
  void OnCommit(unsigned handle, std::string text);
  void OnPreeditChanged(unsigned handle, std::string text, std::string commit);
  void OnPreeditEnd();
  void OnPreeditStart();

  // Implement |BrowserChildProcessObserver|.
  virtual void BrowserChildProcessHostConnected(
    const ChildProcessData& data) OVERRIDE;
  virtual void BrowserChildProcessHostDisconnected(
      const ChildProcessData& data) OVERRIDE;
  virtual void BrowserChildProcessCrashed(
      const ChildProcessData& data) OVERRIDE;

 private:
  void OnMessageReceived(const IPC::Message& message);
  void EstablishChannel();
  void UpdateConnection();
  ui::EventConverterOzoneWayland* dispatcher_;
  RemoteStateChangeHandler* state_handler_;
  DISALLOW_COPY_AND_ASSIGN(OzoneChannelHost);
};

}  // namespace content

#endif  // OZONE_CONTENT_OZONE_CHANNEL_HOST_H_
