// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_IPC_CHILD_PROCESS_OBSERVER_H_
#define OZONE_IMPL_IPC_CHILD_PROCESS_OBSERVER_H_

#include "base/message_loop/message_loop.h"
#include "content/public/browser/browser_child_process_observer.h"
#include "content/public/browser/child_process_data.h"

namespace ozonewayland {

class OzoneDisplay;

// OzoneProcessObserver extends BrowserChildProcessObserver. This class is used
// to observe GPU process being forked or re-spawned for various reasons. This
// can be used to add an ipc filter and listen to any relevant messages coming
// from GpuProcess side.
class OzoneProcessObserver : public content::BrowserChildProcessObserver,
                             public base::MessageLoop::DestructionObserver {
 public:
  explicit OzoneProcessObserver(OzoneDisplay* observer);
  virtual ~OzoneProcessObserver();
  // Implement |BrowserChildProcessObserver|.

  virtual void BrowserChildProcessHostConnected(
    const content::ChildProcessData& data) OVERRIDE;

  // MessageLoop::DestructionObserver overrides.
  virtual void WillDestroyCurrentMessageLoop() OVERRIDE;

 private:
  OzoneDisplay* observer_;
  DISALLOW_COPY_AND_ASSIGN(OzoneProcessObserver);
};

}  // namespace ozonewayland

#endif  // OZONE_IMPL_IPC_CHILD_PROCESS_OBSERVER_H_
