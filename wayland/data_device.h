// Copyright 2015 Igalia S.L.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_DATA_DEVICE_H_
#define OZONE_WAYLAND_DATA_DEVICE_H_

#include <wayland-client.h>

#include <string>

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "ozone/wayland/window.h"

namespace ozonewayland {

class WaylandDataOffer;
class WaylandDisplay;

// This class handles copy-and-paste and drag-and-drop in the GPU process.
class WaylandDataDevice {
 public:
  WaylandDataDevice(WaylandDisplay* display, wl_seat* input_seat);
  ~WaylandDataDevice();

  // DataExchangeHandler implementation
  void RequestDragData(const std::string& mime_type);
  void RequestSelectionData(const std::string& mime_type);
  void DragWillBeAccepted(uint32_t serial, const std::string& mime_type);
  void DragWillBeRejected(uint32_t serial);

 private:
  // wl_data_device_listener callbacks
  static void OnDataOffer(void* data,
                          wl_data_device* data_device,
                          wl_data_offer* id);
  static void OnEnter(void* data,
                      wl_data_device* data_device,
                      uint32_t serial,
                      wl_surface* surface,
                      wl_fixed_t x,
                      wl_fixed_t y,
                      wl_data_offer* id);
  static void OnLeave(void* data, wl_data_device* data_device);
  static void OnMotion(void* data,
                       wl_data_device* data_device,
                       uint32_t time,
                       wl_fixed_t x,
                       wl_fixed_t y);
  static void OnDrop(void* data, wl_data_device* data_device);
  static void OnSelection(void* data,
                          wl_data_device* data_device,
                          wl_data_offer* id);

  // The wl_data_device wrapped by this WaylandDataDevice.
  wl_data_device& data_device_;

  // Used to dispatch events to the browser process.
  WaylandDisplay& display_;

  // There are two separate data offers at a time, the drag offer and the
  // selection offer, each with independent lifetimes. When we receive a new
  // offer, it is not immediately possible to know whether the new offer is the
  // drag offer or the selection offer. This variable is used to store ownership
  // of new data offers temporarily until its identity becomes known.
  scoped_ptr<WaylandDataOffer> new_offer_;

  // Offer to receive data from another process via drag-and-drop, or null if no
  // drag-and-drop from another process is in progress.
  scoped_ptr<WaylandDataOffer> drag_offer_;
  // Window corresponding to |drag_offer_|, null if |drag_offer_| is null.
  WaylandWindow* window_;

  // Offer that holds the most-recent clipboard selection, or null if no
  // clipboard data is available.
  scoped_ptr<WaylandDataOffer> selection_offer_;

  DISALLOW_COPY_AND_ASSIGN(WaylandDataDevice);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_DATA_DEVICE_H_
