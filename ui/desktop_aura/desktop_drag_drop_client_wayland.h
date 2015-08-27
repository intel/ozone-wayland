// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_DESKTOP_AURA_DESKTOP_DRAG_DROP_CLIENT_WAYLAND_H_
#define OZONE_IMPL_DESKTOP_AURA_DESKTOP_DRAG_DROP_CLIENT_WAYLAND_H_

#include <list>
#include <string>
#include <vector>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "ui/aura/window_observer.h"
#include "ui/base/dragdrop/os_exchange_data.h"
#include "ui/gfx/geometry/point.h"
#include "ui/views/views_export.h"
#include "ui/wm/public/drag_drop_client.h"
#include "ui/wm/public/drag_drop_delegate.h"

namespace ui {

class PlatformWindow;

}

namespace views {

class VIEWS_EXPORT DesktopDragDropClientWayland
    : public aura::client::DragDropClient,
      public aura::WindowObserver {
 public:
  explicit DesktopDragDropClientWayland(aura::Window* root_window,
                                        ui::PlatformWindow* platform_window);
  ~DesktopDragDropClientWayland() override;

  // Overridden from aura::client::DragDropClient:
  int StartDragAndDrop(const ui::OSExchangeData& data,
                       aura::Window* root_window,
                       aura::Window* source_window,
                       const gfx::Point& root_location,
                       int operation,
                       ui::DragDropTypes::DragEventSource source) override;
  void DragUpdate(aura::Window* target,
                  const ui::LocatedEvent& event) override;
  void Drop(aura::Window* target,
            const ui::LocatedEvent& event) override;
  void DragCancel() override;
  bool IsDragDropInProgress() override;

  // Overridden from void aura::WindowObserver:
  void OnWindowDestroying(aura::Window* window) override;

  // Events received via IPC from the WaylandDataSource in the GPU process.
  void OnDragEnter(gfx::AcceleratedWidget windowhandle,
                   float x,
                   float y,
                   const std::vector<std::string>& mime_types,
                   uint32_t serial);
  void OnDragDataReceived(int pipefd);
  void OnDragLeave();
  void OnDragMotion(float x, float y, uint32_t time);
  void OnDragDrop();

 private:
  // Used to build up an OSExchangeDataProvider and pass it to the
  // DragDropDelegate of the target window. This requires round-tripping between
  // the browser process and the GPU process once for each type of data to be
  // stored in the OSExchangeDataProvider. The GPU process will pass us a file
  // descriptor to read the data from, then once the read has completed we will
  // close the descriptor and indicate to the GPU process that it is safe to
  // request the next file descriptor. This has to happen one-by-one because
  // some Wayland clients (e.g. GTK+ applications) expect that after calling
  // wl_data_offer_receive() (in the GPU process), we will drain the received
  // file descriptor of all data and close it before calling
  // wl_data_offer_receive() again. We could read from the pipe either
  // asynchronously in the IO thread, or synchronously in the user-blocking file
  // thread. The file thread was chosen for simplicity. Either way, a
  // RefCountedThreadSafe container is required, since there is a very small
  // window in which a DesktopDragDropClientWayland could theoretically be
  // destroyed in the middle of an ongoing drag operation, and the drag could be
  // cancelled before all data has been received. Note also that a
  // DragDataCollector corresponds to exactly one outstanding drag-and-drop
  // and should not be reused for a second drag-and-drop session.
  class DragDataCollector
      : public base::RefCountedThreadSafe<DragDataCollector>,
        public base::SupportsWeakPtr<DragDataCollector> {
   public:
    DragDataCollector(
        base::WeakPtr<DesktopDragDropClientWayland> drag_drop_client,
        const std::vector<std::string>& mime_types,
        gfx::AcceleratedWidget windowhandle);
    // DragDropDataController assumes ownership of |pipefd| and will ensure it
    // is closed exactly once.
    void SetPipeFd(int pipefd);
    int GetPipeFd() const { return pipefd_; }

    std::string first_received_mime_type() const {
      return first_received_mime_type_;
    }

    bool IsReadingData() const;
    void HandleNextMimeType();

    const ui::OSExchangeData& GetData() const { return os_exchange_data_; }

    // Synchronously read drag-and-drop data from the source process.
    static void ReadDragData(scoped_refptr<DragDataCollector>);
   private:
    friend class base::RefCountedThreadSafe<DragDataCollector>;
    ~DragDataCollector();

    // Returns the next MIME type to be received from the source process, or an
    // empty string if there are no more interesting MIME types left to process.
    std::string SelectNextMimeType();

    base::WeakPtr<DesktopDragDropClientWayland> drag_drop_client_;
    ui::OSExchangeData os_exchange_data_;
    std::list<std::string> unprocessed_mime_types_;
    std::string first_received_mime_type_;
    int windowhandle_;
    int pipefd_ = 0;
  };

 public:
  // Called by the DragDataCollector when all drag data has been received.
  void OnDragDataCollected(DragDataCollector* collector);

 private:
  // Returns a DropTargetEvent to be passed to the DragDropDelegate, or null to
  // abort the drag, using the location of point_ if no root location is passed.
  scoped_ptr<ui::DropTargetEvent> CreateDropTargetEvent(
      const gfx::Point& point) const;
  scoped_ptr<ui::DropTargetEvent> CreateDropTargetEvent() const;

  void DragDropSessionCompleted();

  // Use to provide feedback to the source process on whether the drag will be
  // accepted for the current coordinates and available mime types. This might
  // be used to change the cursor, for example.
  void IndicateDragWillBeAccepted();
  void IndicateDragWillBeRejected();

  // Valid between the start of a drag operation and when the process of reading
  // drag-and-drop data from the source process has completed. No drag events
  // can be sent to the DragDropDelegate until all data has been collected.
  scoped_refptr<DragDataCollector> data_collector_;

  // Unique identifier for the current drag.
  uint32_t serial_ = 0;

  // Window handle for active drag, or 0 if there is no active drag.
  unsigned windowhandle_ = 0;

  aura::Window& root_window_;
  ui::PlatformWindow& platform_window_;

  // Null unless all drag data has been received and the drag is unfinished.
  aura::Window* target_window_;

  // The most recent native coordinates of a drag.
  gfx::Point point_;

  // This is the interface that allows us to send drag events from Ozone-Wayland
  // to the cross-platform code.
  aura::client::DragDropDelegate* delegate_;

  // The Wayland sends a drag leave event after a drag drop event, but the
  // DragDropDelegate expects only one or the other. This is set to false after
  // a successful drop to indicate that the drag leave event should not be
  // passed on to the DragDropDelegate.
  bool should_emit_drag_exited_ = true;

  // Used to keep track of whether IndicateDragWillBeAccepted or
  // IndicateDragWillBeRejected was most-recently called.
  bool did_most_recently_accept_drag_ = false;

  // Null except when a drop has occured but drag data is still being read.
  scoped_ptr<gfx::Point> delayed_drop_location_;

  base::WeakPtrFactory<DesktopDragDropClientWayland> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(DesktopDragDropClientWayland);
};

}  // namespace views

#endif  // OZONE_IMPL_DESKTOP_AURA_DESKTOP_DRAG_DROP_CLIENT_WAYLAND_H_
