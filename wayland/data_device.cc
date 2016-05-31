// Copyright 2015 Igalia S.L.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/data_device.h"

#include "ozone/wayland/data_offer.h"
#include "ozone/wayland/display.h"

namespace ozonewayland {

WaylandDataDevice::WaylandDataDevice(WaylandDisplay* display, wl_seat* seat)
    : data_device_(*wl_data_device_manager_get_data_device(
                       display->GetDataDeviceManager(), seat)),
      display_(*display),
      drag_offer_(nullptr),
      window_(nullptr),
      selection_offer_(nullptr) {
  static const struct wl_data_device_listener kDataDeviceListener = {
    WaylandDataDevice::OnDataOffer,
    WaylandDataDevice::OnEnter,
    WaylandDataDevice::OnLeave,
    WaylandDataDevice::OnMotion,
    WaylandDataDevice::OnDrop,
    WaylandDataDevice::OnSelection
  };
  wl_data_device_add_listener(&data_device_, &kDataDeviceListener, this);
}

WaylandDataDevice::~WaylandDataDevice() {
  wl_data_device_destroy(&data_device_);
}

void WaylandDataDevice::RequestDragData(const std::string& mime_type) {
  if (drag_offer_) {
    DCHECK(window_);
    int pipefd = drag_offer_->Receive(mime_type);
    display_.DragData(window_->Handle(), {pipefd, true});
  }
}

void WaylandDataDevice::RequestSelectionData(const std::string& mime_type) {
  NOTIMPLEMENTED();
}

void WaylandDataDevice::DragWillBeAccepted(uint32_t serial,
                                           const std::string& mime_type) {
  if (drag_offer_)
    drag_offer_->Accept(serial, mime_type);
}

void WaylandDataDevice::DragWillBeRejected(uint32_t serial) {
  if (drag_offer_)
    drag_offer_->Reject(serial);
}

// static
void WaylandDataDevice::OnDataOffer(void* data,
                                    wl_data_device* data_device,
                                    wl_data_offer* id) {
  auto self = static_cast<WaylandDataDevice*>(data);

  DCHECK(!self->new_offer_);
  self->new_offer_.reset(new WaylandDataOffer(id));
}

// static
void WaylandDataDevice::OnEnter(void* data,
                                wl_data_device* data_device,
                                uint32_t serial,
                                wl_surface* surface,
                                wl_fixed_t x,
                                wl_fixed_t y,
                                wl_data_offer* id) {
  auto self = static_cast<WaylandDataDevice*>(data);

  DCHECK(self->new_offer_);
  DCHECK(!self->drag_offer_);
  self->drag_offer_ = std::move(self->new_offer_);

  DCHECK(!self->window_);
  self->window_ = static_cast<WaylandWindow*>(
      wl_surface_get_user_data(surface));

  self->display_.DragEnter(self->window_->Handle(),
                           wl_fixed_to_double(x),
                           wl_fixed_to_double(y),
                           self->drag_offer_->GetAvailableMimeTypes(),
                           serial);
}

// static
void WaylandDataDevice::OnLeave(void* data, wl_data_device* data_device) {
  auto self = static_cast<WaylandDataDevice*>(data);
  self->display_.DragLeave(self->window_->Handle());
  self->drag_offer_.reset();
  self->window_ = nullptr;
}

// static
void WaylandDataDevice::OnMotion(void* data,
                                 wl_data_device* data_device,
                                 uint32_t time,
                                 wl_fixed_t x,
                                 wl_fixed_t y) {
  auto self = static_cast<WaylandDataDevice*>(data);
  self->display_.DragMotion(self->window_->Handle(),
                            wl_fixed_to_double(x),
                            wl_fixed_to_double(y),
                            time);
}

// static
void WaylandDataDevice::OnDrop(void* data, wl_data_device* data_device) {
  auto self = static_cast<WaylandDataDevice*>(data);
  self->display_.DragDrop(self->window_->Handle());
}

// static
void WaylandDataDevice::OnSelection(void* data,
                                    wl_data_device* data_device,
                                    wl_data_offer* id) {
  auto self = static_cast<WaylandDataDevice*>(data);

  // id will be null to indicate that the selection is no longer valid, i.e.
  // there is no longer clipboard data available to paste.
  if (!id) {
    self->selection_offer_.reset();
    return;
  }

  DCHECK(self->new_offer_);
  self->selection_offer_ = std::move(self->new_offer_);

  // TODO(mcatanzaro): Get the selection data to the browser process.
  NOTIMPLEMENTED();
}

}  // namespace ozonewayland
