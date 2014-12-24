// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_PUBLIC_OZONE_CHANNEL_HOST_H_
#define OZONE_UI_PUBLIC_OZONE_CHANNEL_HOST_H_

#include <string>

#include "base/memory/shared_memory.h"
#include "ui/events/event_constants.h"
#include "ui/ozone/public/gpu_platform_support_host.h"

namespace ui {
class RemoteStateChangeHandler;
class EventConverterOzoneWayland;
class WindowStateChangeHandler;

class OzoneChannelHost : public GpuPlatformSupportHost {
 public:
  OzoneChannelHost();
  virtual ~OzoneChannelHost();

  void Initialize();
  void ReleaseRemoteStateChangeHandler();
  WindowStateChangeHandler* GetStateChangeHandler() const;

  // GpuPlatformSupportHost:
  void OnChannelEstablished(int host_id, IPC::Sender* sender) override;
  void OnChannelDestroyed(int host_id) override;
  bool OnMessageReceived(const IPC::Message&) override;

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
  void OnKeyNotify(ui::EventType type, unsigned code);
  void OnVirtualKeyNotify(ui::EventType type,
                          uint32_t key,
                          uint32_t modifiers);
  void OnKeyModifiers(uint32_t mods_depressed,
                      uint32_t mods_latched,
                      uint32_t mods_locked,
                      uint32_t group);
  void OnOutputSizeChanged(unsigned width, unsigned height);
  void OnCloseWidget(unsigned handle);
  void OnWindowResized(unsigned handle,
                       unsigned width,
                       unsigned height);
  void OnWindowUnminimized(unsigned handle);
  void OnWindowActivated(unsigned handle);
  void OnWindowDeActivated(unsigned handle);

  void OnCommit(unsigned handle, std::string text);
  void OnPreeditChanged(unsigned handle, std::string text, std::string commit);
  void OnPreeditEnd();
  void OnPreeditStart();
  void OnInitializeXKB(base::SharedMemoryHandle fd, uint32_t size);

 private:
  RemoteStateChangeHandler* state_handler_;
  EventConverterOzoneWayland* event_converter_;
  DISALLOW_COPY_AND_ASSIGN(OzoneChannelHost);
};

}  // namespace ui

#endif  // OZONE_UI_PUBLIC_OZONE_CHANNEL_HOST_H_
