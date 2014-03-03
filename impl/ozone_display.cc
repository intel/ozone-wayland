// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/ozone_display.h"

#include <map>
#include "content/child/child_process.h"
#include "ozone/content/display_channel.h"
#include "ozone/content/display_channel_host.h"
#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ozone/wayland/display.h"

namespace ozonewayland {

OzoneDisplay* OzoneDisplay::instance_ = NULL;

OzoneDisplay* OzoneDisplay::GetInstance() {
  return instance_;
}

OzoneDisplay::OzoneDisplay() : display_(NULL),
    channel_(NULL),
    host_(NULL) {
  instance_ = this;
}

OzoneDisplay::~OzoneDisplay() {
  Terminate();
  instance_ = NULL;
}

bool OzoneDisplay::InitializeHardware() {
  display_ = new WaylandDisplay(WaylandDisplay::RegisterAsNeeded);
  bool initialized_hardware = display_->display() ? true : false;
  if (initialized_hardware && !content::ChildProcess::current()) {
    // In the multi-process mode, DisplayChannel (in GPU process side) is in
    // charge of establishing an IPC channel with DisplayChannelHost (in
    // Browser Process side). At this moment the GPU process is still
    // initializing though, so DisplayChannel cannot establish the connection
    // and need to delay this to later. Therefore post a task to GpuChildThread
    // and let DisplayChannel handle this right after the GPU process is
    // initialized.
    base::MessageLoop::current()->message_loop_proxy()->PostTask(
        FROM_HERE, base::Bind(&OzoneDisplay::DelayedInitialization, this));
  }

  return initialized_hardware;
}

void OzoneDisplay::ShutdownHardware() {
  Terminate();
}

intptr_t OzoneDisplay::GetNativeDisplay() {
  return (intptr_t)display_->display();
}

gfx::AcceleratedWidget OzoneDisplay::GetAcceleratedWidget() {
  static int opaque_handle = 0;
  if (!display_ && !host_)
    host_ = new content::OzoneDisplayChannelHost();

  opaque_handle++;
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(opaque_handle,
                                                              ui::CREATE,
                                                              0,
                                                              0);

  return (gfx::AcceleratedWidget)opaque_handle;
}

gfx::AcceleratedWidget OzoneDisplay::RealizeAcceleratedWidget(
    gfx::AcceleratedWidget w) {
  DCHECK(display_);
  return (gfx::AcceleratedWidget)display_->RealizeAcceleratedWidget(w);
}

void OzoneDisplay::DelayedInitialization(OzoneDisplay* display) {
  display->channel_ = new content::OzoneDisplayChannel();
  display->channel_->Register();
}

void OzoneDisplay::Terminate() {
  delete channel_;
  delete display_;
}

}  // namespace ozonewayland
