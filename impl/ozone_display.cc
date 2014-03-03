// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/ozone_display.h"

#include "ozone/wayland/display.h"

namespace ozonewayland {

OzoneDisplay* OzoneDisplay::instance_ = NULL;

OzoneDisplay* OzoneDisplay::GetInstance() {
  return instance_;
}

OzoneDisplay::OzoneDisplay() : display_(NULL) {
  instance_ = this;
}

OzoneDisplay::~OzoneDisplay() {
  Terminate();
  instance_ = NULL;
}

bool OzoneDisplay::InitializeHardware() {
  display_ = new WaylandDisplay(WaylandDisplay::RegisterAsNeeded);
  return display_->display() ? true : false;
}

void OzoneDisplay::ShutdownHardware() {
  Terminate();
}

intptr_t OzoneDisplay::GetNativeDisplay() {
  return (intptr_t)display_->display();
}

gfx::AcceleratedWidget OzoneDisplay::GetAcceleratedWidget() {
  static int opaque_handle = 0;
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

void OzoneDisplay::Terminate() {
  delete display_;
}

}  // namespace ozonewayland
