// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_PUBLIC_OZONE_CHANNEL_H_
#define OZONE_UI_PUBLIC_OZONE_CHANNEL_H_

#include "base/strings/string16.h"
#include "ozone/ui/events/window_constants.h"
#include "ui/ozone/public/gpu_platform_support.h"

namespace ui {
class RemoteEventDispatcher;

class OzoneChannel : public GpuPlatformSupport {
 public:
  OzoneChannel();
  virtual ~OzoneChannel();
  void InitializeRemoteDispatcher();

  // GpuPlatformSupport:
  virtual void OnChannelEstablished(IPC::Sender* sender) OVERRIDE;
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  void OnWidgetStateChanged(unsigned handleid,
                            ui::WidgetState state,
                            unsigned width,
                            unsigned height);
  void OnWidgetTitleChanged(unsigned widget, base::string16 title);
  void OnWidgetCursorChanged(int cursor_type);
  void OnWidgetAttributesChanged(unsigned widget,
                                 unsigned parent,
                                 unsigned x,
                                 unsigned y,
                                 ui::WidgetType type);
  void OnWidgetImeReset();
  void OnWidgetShowInputPanel();
  void OnWidgetHideInputPanel();

 private:
  RemoteEventDispatcher* event_converter_;
  DISALLOW_COPY_AND_ASSIGN(OzoneChannel);
};

}  // namespace ui

#endif  // OZONE_UI_PUBLIC_OZONE_CHANNEL_H_
