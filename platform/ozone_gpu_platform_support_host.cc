// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/platform/ozone_gpu_platform_support_host.h"

#include "base/trace_event/trace_event.h"
#include "ui/ozone/common/gpu/ozone_gpu_message_params.h"
#include "ui/ozone/common/gpu/ozone_gpu_messages.h"
#include "ui/ozone/platform/drm/host/channel_observer.h"

namespace ui {

OzoneGpuPlatformSupportHost::OzoneGpuPlatformSupportHost() { }

OzoneGpuPlatformSupportHost::~OzoneGpuPlatformSupportHost() {
}

void OzoneGpuPlatformSupportHost::RegisterHandler(
    GpuPlatformSupportHost* handler) {
  handlers_.push_back(handler);

  if (IsConnected())
    handler->OnChannelEstablished(host_id_, send_runner_, send_callback_);
}

void OzoneGpuPlatformSupportHost::UnregisterHandler(
    GpuPlatformSupportHost* handler) {
  std::vector<GpuPlatformSupportHost*>::iterator it =
      std::find(handlers_.begin(), handlers_.end(), handler);
  if (it != handlers_.end())
    handlers_.erase(it);
}

void OzoneGpuPlatformSupportHost::AddChannelObserver(
    ChannelObserver* observer) {
  channel_observers_.AddObserver(observer);

  if (IsConnected())
    observer->OnChannelEstablished();
}

void OzoneGpuPlatformSupportHost::RemoveChannelObserver(
    ChannelObserver* observer) {
  channel_observers_.RemoveObserver(observer);
}

bool OzoneGpuPlatformSupportHost::IsConnected() {
  return host_id_ >= 0;
}

void OzoneGpuPlatformSupportHost::OnChannelEstablished(
    int host_id,
    scoped_refptr<base::SingleThreadTaskRunner> send_runner,
    const base::Callback<void(IPC::Message*)>& send_callback) {
  TRACE_EVENT1("drm", "OzoneGpuPlatformSupportHost::OnChannelEstablished",
               "host_id", host_id);
  host_id_ = host_id;
  send_runner_ = send_runner;
  send_callback_ = send_callback;

  for (size_t i = 0; i < handlers_.size(); ++i)
    handlers_[i]->OnChannelEstablished(host_id, send_runner_, send_callback_);

  FOR_EACH_OBSERVER(ChannelObserver, channel_observers_,
                    OnChannelEstablished());
}

void OzoneGpuPlatformSupportHost::OnChannelDestroyed(int host_id) {
  TRACE_EVENT1("drm", "OzoneGpuPlatformSupportHost::OnChannelDestroyed",
               "host_id", host_id);
  if (host_id_ == host_id) {
    host_id_ = -1;
    send_runner_ = nullptr;
    send_callback_.Reset();
    FOR_EACH_OBSERVER(ChannelObserver, channel_observers_,
                      OnChannelDestroyed());
  }

  for (size_t i = 0; i < handlers_.size(); ++i)
    handlers_[i]->OnChannelDestroyed(host_id);
}

bool OzoneGpuPlatformSupportHost::OnMessageReceived(
    const IPC::Message& message) {
  for (size_t i = 0; i < handlers_.size(); ++i)
    if (handlers_[i]->OnMessageReceived(message))
      return true;

  return false;
}

bool OzoneGpuPlatformSupportHost::Send(IPC::Message* message) {
  if (IsConnected() &&
      send_runner_->PostTask(FROM_HERE, base::Bind(send_callback_, message)))
    return true;

  delete message;
  return false;
}

}  // namespace ui
