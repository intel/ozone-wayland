// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/desktop_drag_drop_client_wayland.h"

#include "ui/base/dragdrop/drag_drop_types.h"
#include "ui/base/dragdrop/drop_target_event.h"

namespace ozonewayland {

DesktopDragDropClientWayland::DesktopDragDropClientWayland(
    aura::Window* root_window) {
  NOTIMPLEMENTED();
}

DesktopDragDropClientWayland::~DesktopDragDropClientWayland() {
}

int DesktopDragDropClientWayland::StartDragAndDrop(
    const ui::OSExchangeData& data,
    aura::Window* root_window,
    aura::Window* source_window,
    const gfx::Point& root_location,
    int operation,
    ui::DragDropTypes::DragEventSource source) {
  NOTIMPLEMENTED();
  return false;
}

void DesktopDragDropClientWayland::DragUpdate(aura::Window* target,
                                              const ui::LocatedEvent& event) {
  NOTIMPLEMENTED();
}

void DesktopDragDropClientWayland::Drop(aura::Window* target,
                                        const ui::LocatedEvent& event) {
  NOTIMPLEMENTED();
}

void DesktopDragDropClientWayland::DragCancel() {
  NOTIMPLEMENTED();
}

bool DesktopDragDropClientWayland::IsDragDropInProgress() {
  return false;
}

}  // namespace ozonewayland
