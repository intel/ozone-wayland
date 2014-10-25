// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_PUBLIC_OZONE_CHANNEL_HOST_H_
#define OZONE_UI_PUBLIC_OZONE_CHANNEL_HOST_H_

#include <string>

#include "ui/events/event_constants.h"
#include "ui/ozone/public/gpu_platform_support_host.h"

namespace ui {
class RemoteStateChangeHandler;
class EventConverterInProcess;

class OzoneChannelHost : public GpuPlatformSupportHost {
 public:
  OzoneChannelHost();
  virtual ~OzoneChannelHost();

  void DeleteRemoteStateChangeHandler();

  // GpuPlatformSupportHost:
  virtual void OnChannelEstablished(int host_id, IPC::Sender* sender) override;
  virtual void OnChannelDestroyed(int host_id) override;
  virtual bool OnMessageReceived(const IPC::Message&) override;

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
  void OnWindowUnminimized(unsigned handle);
  void OnCommit(unsigned handle, std::string text);
  void OnPreeditChanged(unsigned handle, std::string text, std::string commit);
  void OnPreeditEnd();
  void OnPreeditStart();

 private:
  RemoteStateChangeHandler* state_handler_;
  EventConverterInProcess* event_converter_;
  DISALLOW_COPY_AND_ASSIGN(OzoneChannelHost);
};

}  // namespace ui

#endif  // OZONE_UI_PUBLIC_OZONE_CHANNEL_HOST_H_
