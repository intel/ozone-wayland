// Copyright 2015 Igalia S.L.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_DATA_OFFER_H_
#define OZONE_WAYLAND_DATA_OFFER_H_

#include <wayland-client.h>

#include <string>
#include <vector>

#include "base/macros.h"

namespace ozonewayland {

// The WaylandDataOffer represents copy-and-paste or drag-and-drop data sent to
// us by some Wayland client (possibly ourself).
class WaylandDataOffer {
 public:
  // Takes ownership of data_offer.
  explicit WaylandDataOffer(wl_data_offer* data_offer);

  ~WaylandDataOffer();

  const std::vector<std::string>&
  GetAvailableMimeTypes() const { return mime_types_; }

  // Indicate that |mime_type| can be accepted for the current (x, y) location
  // of the drag. This might be used to change the cursor to indicate whether
  // the drop will be accepted, for example. Does not cause any data to be
  // transferred. |serial| is the value passed to WaylandDataDevice::OnEnter.
  void Accept(uint32_t serial, const std::string& mime_type);
  void Reject(uint32_t serial);

  // Receive data of type mime_type from another Wayland client. Returns an open
  // file descriptor to read the data from, or -1 on failure.
  int Receive(const std::string& mime_type);

 private:
  static void OnOffer(void* data,
                      wl_data_offer* data_offer,
                      const char* mime_type);

  wl_data_offer& data_offer_;
  std::vector<std::string> mime_types_;

  DISALLOW_COPY_AND_ASSIGN(WaylandDataOffer);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_DATA_OFFER_H_
