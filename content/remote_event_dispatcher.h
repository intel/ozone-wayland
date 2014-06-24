// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_CONTENT_REMOTE_EVENT_DISPATCHER_H_
#define OZONE_CONTENT_REMOTE_EVENT_DISPATCHER_H_

#include <string>

#include "content/browser/gpu/gpu_process_host.h"
#include "ozone/ui/events/event_converter_ozone_wayland.h"

namespace content {

// RemoteEventDispatcher sends native events from GPU to Browser process over
// IPC. In Multi-process case, callbacks from Wayland are received in GPU
// process side. All callbacks related to input need to be handled in Browser
// process and hence the events are sent to it over IPC.
class RemoteEventDispatcher : public ui::EventConverterOzoneWayland {
 public:
  RemoteEventDispatcher();
  virtual ~RemoteEventDispatcher();

  void ChannelEstablished(IPC::Sender* sender);

  virtual void MotionNotify(float x, float y) OVERRIDE;
  virtual void ButtonNotify(unsigned handle,
                            ui::EventType type,
                            ui::EventFlags flags,
                            float x,
                            float y) OVERRIDE;
  virtual void AxisNotify(float x,
                          float y,
                          int xoffset,
                          int yoffset) OVERRIDE;
  virtual void PointerEnter(unsigned handle, float x, float y) OVERRIDE;
  virtual void PointerLeave(unsigned handle, float x, float y) OVERRIDE;
  virtual void KeyNotify(ui::EventType type,
                         unsigned code,
                         unsigned modifiers) OVERRIDE;
  virtual void TouchNotify(ui::EventType type,
                           float x,
                           float y,
                           int32_t touch_id,
                           uint32_t time_stamp) OVERRIDE;

  virtual void OutputSizeChanged(unsigned width, unsigned height) OVERRIDE;
  virtual void WindowResized(unsigned handle,
                             unsigned width,
                             unsigned height) OVERRIDE;
  virtual void CloseWidget(unsigned handle) OVERRIDE;

  virtual void Commit(unsigned handle, const std::string& text) OVERRIDE;
  virtual void PreeditChanged(unsigned handle,
                              const std::string& text,
                              const std::string& commit) OVERRIDE;
  virtual void PreeditEnd() OVERRIDE;
  virtual void PreeditStart() OVERRIDE;

 private:
  void Dispatch(IPC::Message* message);
  static void Send(RemoteEventDispatcher* dispatcher,
                   IPC::Message* message);
  IPC::Sender* sender_;
  DISALLOW_COPY_AND_ASSIGN(RemoteEventDispatcher);
};

}  // namespace content

#endif  // OZONE_CONTENT_REMOTE_EVENT_DISPATCHER_H_
