// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_WAYLAND_CHILD_PROCESS_OBSERVER
#define OZONE_WAYLAND_CHILD_PROCESS_OBSERVER

#include "content/public/browser/browser_child_process_observer.h"
#include "content/public/browser/child_process_data.h"

namespace ozonewayland {

class OzoneDisplay;

// OzoneProcessObserver extends BrowserChildProcessObserver. This class can be
// used to observe when browserprocess forks child process i.e GPU and when child
// process gets closed. This can be used to add an ipc filter and listen to any
// relevant messages coming from GpuProcess side.
class OzoneProcessObserver : public content::BrowserChildProcessObserver {
 public:
  OzoneProcessObserver(OzoneDisplay* observer);
  virtual ~OzoneProcessObserver();
  // Implement |BrowserChildProcessObserver|.
  virtual void BrowserChildProcessHostDisconnected(
    const content::ChildProcessData& data) OVERRIDE;

  virtual void BrowserChildProcessCrashed(
    const content::ChildProcessData& data) OVERRIDE;

  virtual void BrowserChildProcessHostConnected(
    const content::ChildProcessData& data) OVERRIDE;

  void WillDestroyCurrentMessageLoop();

 private:
  OzoneDisplay* observer_;
  DISALLOW_COPY_AND_ASSIGN(OzoneProcessObserver);
};

}  // namespace ozonewayland

#endif  // OZONE_WAYLAND_CHILD_PROCESS_OBSERVER
