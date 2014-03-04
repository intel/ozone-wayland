// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/ozone_hardware_display.h"

#include "ozone/wayland/display.h"

namespace ozonewayland {

OzoneHardwareDisplay::OzoneHardwareDisplay() : native_display_(NULL) {
  gfx::OzoneDisplay::SetInstance(this);
}

OzoneHardwareDisplay::~OzoneHardwareDisplay() {
  Terminate();
  gfx::OzoneDisplay::SetInstance(NULL);
}

gfx::SurfaceFactoryOzone::HardwareState
OzoneHardwareDisplay::InitializeHardware() {
  if (!native_display_) {
    native_display_ = new WaylandDisplay(WaylandDisplay::RegisterAsNeeded);
    if (!native_display_->display()) {
      delete native_display_;
      native_display_ = NULL;
      LOG(ERROR) << "SurfaceFactoryWayland failed to initialize hardware";
      return gfx::SurfaceFactoryOzone::FAILED;
    }
  }

  return gfx::SurfaceFactoryOzone::INITIALIZED;
}

void OzoneHardwareDisplay::ShutdownHardware() {
  Terminate();
}

intptr_t OzoneHardwareDisplay::GetNativeDisplay() {
  return (intptr_t)native_display_->display();
}

void OzoneHardwareDisplay::FlushDisplay() {
  DCHECK(native_display_);
  native_display_->FlushDisplay();
}

gfx::AcceleratedWidget OzoneHardwareDisplay::GetAcceleratedWidget() {
  static int opaque_handle = 0;
  opaque_handle++;
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(opaque_handle,
                                                              ui::CREATE,
                                                              0,
                                                              0);

  return (gfx::AcceleratedWidget)opaque_handle;
}

gfx::AcceleratedWidget OzoneHardwareDisplay::RealizeAcceleratedWidget(
    gfx::AcceleratedWidget w) {
  DCHECK(native_display_);
  return (gfx::AcceleratedWidget)native_display_->RealizeAcceleratedWidget(w);
}

bool OzoneHardwareDisplay::AttemptToResizeAcceleratedWidget(
    gfx::AcceleratedWidget w, const gfx::Rect& bounds) {
    ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(
        w, ui::RESIZE, bounds.width(), bounds.height());

    return true;
}

void OzoneHardwareDisplay::DestroyWidget(gfx::AcceleratedWidget w) {
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(w,
                                                              ui::DESTROYED);
}

void OzoneHardwareDisplay::LookAheadOutputGeometry() {
  WaylandDisplay::LookAheadOutputGeometry();
}

void OzoneHardwareDisplay::Terminate() {
  delete native_display_;
}

}  // namespace ozonewayland
