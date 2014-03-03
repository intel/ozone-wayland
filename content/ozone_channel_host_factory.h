// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_CONTENT_OZONE_CHANNEL_HOST_FACTORY_H_
#define OZONE_CONTENT_OZONE_CHANNEL_HOST_FACTORY_H_

#include "ozone/content/display_channel_host.h"

namespace ui {
class EventConverterInProcess;
}

namespace content {
class OzoneDisplayChannelHost;
// This class is responsible for establishing gpu channel connection between
// OzoneDisplayChannel and ChannelHost.
class OzoneChannelHostFactory {
 public:
  static void Initialize(bool establish_gpu_channel);
  static void Terminate();
  static OzoneChannelHostFactory* instance();

  explicit OzoneChannelHostFactory(bool establish_gpu_channel);
  virtual ~OzoneChannelHostFactory();

 private:
  ui::EventConverterInProcess* event_converter_;
  OzoneDisplayChannelHost* host_;
  static OzoneChannelHostFactory* instance_;
  DISALLOW_COPY_AND_ASSIGN(OzoneChannelHostFactory);
};

}  // namespace content

#endif  // OZONE_CONTENT_OZONE_CHANNEL_HOST_FACTORY_H_
