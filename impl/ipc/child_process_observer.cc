// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/ipc/child_process_observer.h"

#include "ozone/impl/ozone_display.h"
#include "content/public/common/process_type.h"

namespace ozonewayland {

OzoneProcessObserver::OzoneProcessObserver(OzoneDisplay* observer)
    : observer_(observer)
{
  BrowserChildProcessObserver::Add(this);
}

OzoneProcessObserver::~OzoneProcessObserver()
{
}

void OzoneProcessObserver::BrowserChildProcessHostDisconnected(
  const content::ChildProcessData& data)
{
  observer_->OnChannelClosed();
}

void OzoneProcessObserver::BrowserChildProcessCrashed(
  const content::ChildProcessData& data)
{
  observer_->OnChannelClosed();
}

void OzoneProcessObserver::BrowserChildProcessHostConnected(
  const content::ChildProcessData& data)
{
}

void OzoneProcessObserver::WillDestroyCurrentMessageLoop()
{
  BrowserChildProcessObserver::Remove(this);
}

}  // namespace ozonewayland
