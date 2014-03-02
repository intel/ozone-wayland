// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/content/ozone_channel_host_factory.h"

#include "ozone/content/display_channel_host.h"
#include "ozone/content/event_converter_in_process.h"
#include "ozone/ui/events/event_factory_ozone_wayland.h"

namespace content {
OzoneChannelHostFactory* OzoneChannelHostFactory::instance_ = NULL;

void OzoneChannelHostFactory::Initialize(bool establish_gpu_channel) {
  DCHECK(!instance_);
  instance_ = new OzoneChannelHostFactory(establish_gpu_channel);
}

void OzoneChannelHostFactory::Terminate() {
  DCHECK(instance_);
  delete instance_;
  instance_ = NULL;
}

OzoneChannelHostFactory* OzoneChannelHostFactory::instance() {
  return instance_;
}

OzoneChannelHostFactory::OzoneChannelHostFactory(bool establish_gpu_channel)
    : event_converter_(NULL) {
  event_converter_ = new EventConverterInProcess();
  ui::EventFactoryOzoneWayland* event_factory =
    ui::EventFactoryOzoneWayland::GetInstance();
  event_factory->SetEventConverterOzoneWayland(event_converter_);
  event_converter_->SetWindowChangeObserver(
      event_factory->GetWindowChangeObserver());
  event_converter_->SetOutputChangeObserver(
      event_factory->GetOutputChangeObserver());
  if (establish_gpu_channel)
    host_ = new OzoneDisplayChannelHost();
}

OzoneChannelHostFactory::~OzoneChannelHostFactory() {
  ui::EventFactoryOzoneWayland::GetInstance()->
      SetEventConverterOzoneWayland(NULL);
  delete event_converter_;
}

}  // namespace content
