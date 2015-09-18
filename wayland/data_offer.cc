// Copyright 2015 Igalia S.L.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/data_offer.h"

#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

#include "base/logging.h"

namespace ozonewayland {

WaylandDataOffer::WaylandDataOffer(wl_data_offer* data_offer)
    : data_offer_(*data_offer) {
  static const struct wl_data_offer_listener kDataOfferListener = {
    WaylandDataOffer::OnOffer
  };
  wl_data_offer_add_listener(data_offer, &kDataOfferListener, this);
}

WaylandDataOffer::~WaylandDataOffer() {
  wl_data_offer_destroy(&data_offer_);
}

void WaylandDataOffer::Accept(uint32_t serial, const std::string& mime_type) {
  wl_data_offer_accept(&data_offer_, serial, mime_type.data());
}

void WaylandDataOffer::Reject(uint32_t serial) {
  // Passing a null MIME type means "reject."
  wl_data_offer_accept(&data_offer_, serial, nullptr);
}

int WaylandDataOffer::Receive(const std::string& mime_type) {
  int pipefd[2];
  if (pipe2(pipefd, O_CLOEXEC) == -1) {
    LOG(ERROR) << "Failed to create pipe: " << strerror(errno);
    return -1;
  }

  wl_data_offer_receive(&data_offer_, mime_type.data(), pipefd[1]);
  close(pipefd[1]);
  return pipefd[0];
}

// static
void WaylandDataOffer::OnOffer(void* data,
                               wl_data_offer* data_offer,
                               const char* mime_type) {
  auto self = static_cast<WaylandDataOffer*>(data);
  self->mime_types_.push_back(mime_type);
}

}  // namespace ozonewayland
