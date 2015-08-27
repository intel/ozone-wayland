// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/desktop_aura/desktop_drag_drop_client_wayland.h"

#include <algorithm>
#include <iterator>

#include "base/files/file_path.h"
#include "base/strings/string16.h"
#include "base/strings/string_split.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "content/public/browser/browser_thread.h"
#include "ui/aura/window.h"
#include "ui/aura/window_tree_host.h"
#include "ui/base/clipboard/clipboard.h"
#include "ui/base/dragdrop/drag_drop_types.h"
#include "ui/base/dragdrop/drop_target_event.h"
#include "ui/base/dragdrop/file_info.h"
#include "ui/base/dragdrop/os_exchange_data_provider_aura.h"
#include "ui/platform_window/platform_window.h"

// TODO(mcatanzaro): Add support for accepting drags from GTK+ and Qt.
// Currently, only drags from the reference weston-dnd client are supported.
//
// We will need changes in both Wayland and GTK+ to support drags from GTK+.
//
// See: https://bugs.freedesktop.org/show_bug.cgi?id=91944
//
// Also relevant: https://bugs.freedesktop.org/show_bug.cgi?id=91945
//
// I have not tested Qt yet.

// TODO(mcatanzaro): Add support for drags originating from Chrome. Currently,
// only drags into Chrome from another process are supported.

namespace views {

namespace {

auto& kMimeTypeText = ui::Clipboard::kMimeTypeText;
auto& kMimeTypeURIList = ui::Clipboard::kMimeTypeURIList;

const char kMimeTypeTextUTF8[] = "text/plain;charset=utf-8";

void AddStringToOSExchangeData(ui::OSExchangeData* os_exchange_data,
                               const std::string& data) {
  if (data.empty())
    return;

  base::string16 string16 = base::UTF8ToUTF16(data);
  os_exchange_data->SetString(string16);

  VLOG(1) << "Added string " << string16 << " to OSExchangeData";
}

void AddURIListToOSExchangeData(ui::OSExchangeData* os_exchange_data,
                                const std::string& data) {
  std::vector<std::string> filenames = base::SplitString(
      data,
      "\n",
      base::TRIM_WHITESPACE,
      base::SPLIT_WANT_NONEMPTY);

  // The URI list is allowed to contain comments, which need to be removed.
  auto new_end = std::remove_if(filenames.begin(), filenames.end(),
                                [](const std::string& value) {
                                    return value.empty() || value[0] == '#';
                                });
  filenames.erase(new_end, filenames.end());

  if (filenames.empty())
    return;

  std::vector<ui::FileInfo> file_infos;
  for (std::string& filename : filenames) {
    ui::FileInfo file;
    file.path = base::FilePath::FromUTF8Unsafe(filename);
    file_infos.push_back(file);
    VLOG(1) << "Adding filename " << filename << " to OSExchangeData";
  }

  os_exchange_data->SetFilenames(file_infos);
}

void AddToOSExchangeData(ui::OSExchangeData* os_exchange_data,
                         const std::string& data,
                         const std::string& mime_type) {
  VLOG(2) <<  __FUNCTION__ << " data=" << data << " mime_type=" << mime_type;

  if ((mime_type == kMimeTypeText || mime_type == kMimeTypeTextUTF8)) {
    DCHECK(!os_exchange_data->HasString());
    AddStringToOSExchangeData(os_exchange_data, data);
    return;
  }

  if (mime_type == kMimeTypeURIList) {
    DCHECK(!os_exchange_data->HasFile());
    AddURIListToOSExchangeData(os_exchange_data, data);
    return;
  }

  NOTREACHED();
}

}  // namespace

DesktopDragDropClientWayland::DragDataCollector::DragDataCollector(
    base::WeakPtr<DesktopDragDropClientWayland> drag_drop_client,
    const std::vector<std::string>& mime_types,
    gfx::AcceleratedWidget windowhandle)
    : drag_drop_client_(drag_drop_client),
      os_exchange_data_(new ui::OSExchangeDataProviderAura),
      windowhandle_(windowhandle) {
  std::copy(mime_types.begin(),
            mime_types.end(),
            std::insert_iterator<std::list<std::string>>(
                unprocessed_mime_types_,
                unprocessed_mime_types_.begin()));
}

DesktopDragDropClientWayland::DragDataCollector::~DragDataCollector() {
  if (pipefd_)
    close(pipefd_);
}

void DesktopDragDropClientWayland::DragDataCollector::SetPipeFd(int pipefd) {
  DCHECK(!IsReadingData());
  pipefd_ = pipefd;
}

bool DesktopDragDropClientWayland::DragDataCollector::IsReadingData() const {
  return !!pipefd_;
}

void DesktopDragDropClientWayland::DragDataCollector::HandleNextMimeType() {
  DCHECK(!IsReadingData());

  if (!drag_drop_client_)
    return;

  std::string mime_type = SelectNextMimeType();
  if (!mime_type.empty()) {
    VLOG(1) << __FUNCTION__ << ": requesting data of MIME type " << mime_type;
    drag_drop_client_->platform_window_.RequestDragData(mime_type);
  } else {
    drag_drop_client_->OnDragDataCollected(this);
  }
}

// static
void DesktopDragDropClientWayland::DragDataCollector::ReadDragData(
    scoped_refptr<DragDataCollector> data_collector) {
  VLOG(1) <<  __FUNCTION__ << " data_collector=" << data_collector.get();

  base::ThreadRestrictions::AssertIOAllowed();

  ssize_t bytes_read;
  std::string data;
  for (;;) {
    char buffer[PIPE_BUF + 1];
    bytes_read = HANDLE_EINTR(read(data_collector->GetPipeFd(),
                                   buffer,
                                   sizeof(buffer) - 1));
    if (bytes_read == 0)
      break;
    if (bytes_read == -1)
      break;
    buffer[bytes_read] = '\0';
    data += buffer;
  }
  close(data_collector->pipefd_);
  data_collector->pipefd_ = 0;

  DCHECK(!data_collector->unprocessed_mime_types_.empty());

  if (!data.empty()) {
    AddToOSExchangeData(&data_collector->os_exchange_data_,
                        data,
                        data_collector->unprocessed_mime_types_.front());

    if (data_collector->first_received_mime_type_.empty()) {
      data_collector->first_received_mime_type_ =
          data_collector->unprocessed_mime_types_.front();
    }
  }

  data_collector->unprocessed_mime_types_.erase(
      data_collector->unprocessed_mime_types_.begin());

  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(
          &DesktopDragDropClientWayland::DragDataCollector::HandleNextMimeType,
          data_collector.get()->AsWeakPtr()));
}

std::string
DesktopDragDropClientWayland::DragDataCollector::SelectNextMimeType() {
  // unprocessed_mime_types_ is a list instead of a vector to avoid repeatedly
  // reallocating a vector when removing its first element in here. The
  // performance impact would normally be insignificant, but it might matter if
  // a malicious Wayland client offers a huge list of MIME types. We don't want
  // to start from the last element of the list so as to prefer MIME types that
  // occur earlier in the list.
  while (!unprocessed_mime_types_.empty()) {
    std::string& mime_type = unprocessed_mime_types_.front();
    if (((mime_type == kMimeTypeText || mime_type == kMimeTypeTextUTF8) &&
         !os_exchange_data_.HasString()) ||
        (mime_type == kMimeTypeURIList && !os_exchange_data_.HasFile()))  {
      return mime_type;
    }
    unprocessed_mime_types_.erase(unprocessed_mime_types_.begin());
  }
  return std::string();
}

DesktopDragDropClientWayland::DesktopDragDropClientWayland(
    aura::Window* root_window, ui::PlatformWindow* platform_window)
    : root_window_(*root_window),
      platform_window_(*platform_window),
      target_window_(nullptr),
      delegate_(nullptr),
      delayed_drop_location_(nullptr),
      weak_ptr_factory_(this) {
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
  return !!windowhandle_;
}

void DesktopDragDropClientWayland::OnWindowDestroying(aura::Window* window) {
  DCHECK_EQ(target_window_, window);
  target_window_->RemoveObserver(this);
  target_window_ = nullptr;
}

void DesktopDragDropClientWayland::OnDragEnter(
    gfx::AcceleratedWidget windowhandle,
    float x,
    float y,
    const std::vector<std::string>& mime_types,
    uint32_t serial) {
  VLOG(1) <<  __FUNCTION__ << " windowhandle=" << windowhandle
           << " x=" << x << " y=" << y << " serial=" << serial;

  if (data_collector_) {
    // A previous drag ended before drag data was fully transferred, then this
    // new drag started. Cancel the old drag before proceeding.
    DragDropSessionCompleted();
  }

  if (windowhandle_) {
    LOG(ERROR) << "Received DragEnter event during an outstanding drag.";
    return;
  }

  if (!windowhandle || mime_types.empty()) {
    LOG(ERROR) << "Received invalid DragEnter event from GPU process.";
    return;
  }

  point_.SetPoint(x, y);

  serial_ = serial;
  windowhandle_ = windowhandle;

  data_collector_ = new DragDataCollector(weak_ptr_factory_.GetWeakPtr(),
                                          mime_types,
                                          windowhandle);
  data_collector_->HandleNextMimeType();

  // From here on out, it's unsafe to modify the DragDataCollector from the
  // browser thread until the DragDataCollector calls OnDragDataCollected.
}

void DesktopDragDropClientWayland::OnDragDataReceived(int pipefd) {
  VLOG(1) <<  __FUNCTION__ << " pipefd=" << pipefd;

  if (!data_collector_) {
    // OnDragLeave has already been called.
    close(pipefd);
    return;
  }

  if (data_collector_->IsReadingData()) {
    LOG(ERROR) << "Received DragData event during an outstanding read.";
    close(pipefd);
    return;
  }

  data_collector_->SetPipeFd(pipefd);
  content::BrowserThread::PostTask(
      content::BrowserThread::FILE_USER_BLOCKING,
      FROM_HERE,
      base::Bind(&DesktopDragDropClientWayland::DragDataCollector::ReadDragData,
                 data_collector_));
}

void DesktopDragDropClientWayland::OnDragLeave() {
  VLOG(1) <<  __FUNCTION__;

  // Wayland sends OnDragLeave after OnDragDrop, but the DragDropDelegate
  // expects to receive only one or the other.
  if (delegate_ && should_emit_drag_exited_)
    delegate_->OnDragExited();

  should_emit_drag_exited_ = true;

  if (!delayed_drop_location_) {
    DragDropSessionCompleted();
  }

  // TODO(mcatanzaro): It would be nice to support cancellation of the previous
  // DragDataCollector's tasks. Otherwise, it's just going to keep on
  // reading data until it finishes, blocking its thread. But this probably
  // doesn't matter much, since the reading will usually be pretty fast.
}

void DesktopDragDropClientWayland::OnDragMotion(float x,
                                                float y,
                                                uint32_t time) {
  VLOG(2) << __FUNCTION__ << " x=" << x << " y=" << y << " time=" << time;

  point_.SetPoint(x, y);

  if (!delegate_)
    return;

  scoped_ptr<ui::DropTargetEvent> event = CreateDropTargetEvent();
  if (event) {
    bool shouldAcceptDrag = delegate_->OnDragUpdated(*event);

    if (shouldAcceptDrag && !did_most_recently_accept_drag_)
      IndicateDragWillBeAccepted();
    else if (!shouldAcceptDrag && did_most_recently_accept_drag_)
      IndicateDragWillBeRejected();
  }
}

void DesktopDragDropClientWayland::OnDragDrop() {
  VLOG(1) <<  __FUNCTION__;

  if (delegate_) {
    scoped_ptr<ui::DropTargetEvent> event = CreateDropTargetEvent();
    // All the drag data is available. This is the usual case.
    delegate_->OnPerformDrop(*event);
    should_emit_drag_exited_ = false;
  } else if (target_window_) {
    // Unlikely case: the drop could occur before all the drag data has been
    // transferred from the source process. If so, we must delay the emission
    // of the drop event until the data is available.
    delayed_drop_location_.reset(new gfx::Point(point_.x(), point_.y()));
    root_window_.GetHost()->ConvertPointFromNativeScreen(
        delayed_drop_location_.get());
  }
}

void DesktopDragDropClientWayland::IndicateDragWillBeAccepted() {
  DCHECK(serial_);
  DCHECK(data_collector_);
  DCHECK(!data_collector_->first_received_mime_type().empty());

  // TODO(mcatanzaro): https://bugs.freedesktop.org/show_bug.cgi?id=91950
  // It is impossible to know which MIME type to accept, since DragDropDelegate
  // does not provide this information. Since this won't affect the
  // correctness of the drag (it is only to allow the source client to show a
  // type-dependent cursor), we just send the first MIME type that we received
  // data for. It would be better if Wayland would provide a way to indicate
  // acceptance without specifying MIME type.
  platform_window_.DragWillBeAccepted(
      serial_,
      data_collector_->first_received_mime_type());

  did_most_recently_accept_drag_ = true;
}

void DesktopDragDropClientWayland::IndicateDragWillBeRejected() {
  DCHECK(serial_);
  platform_window_.DragWillBeRejected(serial_);

  did_most_recently_accept_drag_ = false;
}

void DesktopDragDropClientWayland::OnDragDataCollected(
    DragDataCollector* collector) {
  VLOG(1) <<  __FUNCTION__ << " collector=" << collector;

  // Make sure the drag has not already been cancelled. If it has been
  // cancelled, then |data_collector_| will have been reset by OnDragLeave()
  // and will not match |collector|.
  if (data_collector_ != collector)
    return;

  // Did the other process actually send data?
  if (collector->first_received_mime_type().empty()) {
    IndicateDragWillBeRejected();
    return;
  }

  gfx::Point root_location(point_.x(), point_.y());
  root_window_.GetHost()->ConvertPointFromNativeScreen(&root_location);

  target_window_ = root_window_.GetEventHandlerForPoint(root_location);
  if (!target_window_)
    return;

  target_window_->AddObserver(this);

  delegate_ = aura::client::GetDragDropDelegate(target_window_);
  if (!delegate_)
    return;

  scoped_ptr<ui::DropTargetEvent> event = CreateDropTargetEvent();
  if (event) {
    VLOG(1) << "Sending OnDragEntered to DragDropDelegate";
    delegate_->OnDragEntered(*event);
    delegate_->OnDragUpdated(*event) ?
        IndicateDragWillBeAccepted() : IndicateDragWillBeRejected();

    if (delayed_drop_location_) {
      // The drop has already occurred, and should be reported immediately. In
      // this case, the coordinates reported for the drop are actually from an
      // earlier point in time than the coordinates reported for the drag enter,
      // which is weird, but harmless: it's only important that the drop occur
      // at the right place.
      scoped_ptr<ui::DropTargetEvent> delayedEvent =
          CreateDropTargetEvent(*delayed_drop_location_.get());
      delegate_->OnPerformDrop(*delayedEvent.get());
      DragDropSessionCompleted();
      delayed_drop_location_.reset();
    }
  }
}

scoped_ptr<ui::DropTargetEvent>
DesktopDragDropClientWayland::CreateDropTargetEvent(
    const gfx::Point& root_location) const {
  if (!target_window_)
    return nullptr;

  // TODO(mcatanzaro): The Wayland drag-and-drop protocol is not yet
  // sufficiently advanced to allow specifying drag actions (move/copy/link),
  // so we have no choice but to hardcode one drag action here.
  // https://bugs.freedesktop.org/show_bug.cgi?id=91949
  int drag_operations = ui::DragDropTypes::DRAG_COPY;

  gfx::Point target_location = root_location;
  target_window_->GetHost()->ConvertPointToHost(&target_location);
  aura::Window::ConvertPointToTarget(&root_window_,
                                     target_window_,
                                     &target_location);

  return scoped_ptr<ui::DropTargetEvent>(new ui::DropTargetEvent(
      data_collector_->GetData(),
      target_location,
      root_location,
      drag_operations));
}

scoped_ptr<ui::DropTargetEvent>
DesktopDragDropClientWayland::CreateDropTargetEvent() const {
  gfx::Point root_location(point_.x(), point_.y());
  root_window_.GetHost()->ConvertPointFromNativeScreen(&root_location);

  return CreateDropTargetEvent(root_location);
}

void DesktopDragDropClientWayland::DragDropSessionCompleted() {
  serial_ = 0;
  windowhandle_ = 0;
  data_collector_ = nullptr;
  if (target_window_) {
    target_window_->RemoveObserver(this);
    target_window_ = nullptr;
  }
}

}  // namespace views
