// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/wayland/display.h"

#include <EGL/egl.h>
#include <errno.h>
#include <fcntl.h>
#if defined(ENABLE_DRM_SUPPORT)
#include <gbm.h>
#include <libdrm/drm.h>
#include <xf86drm.h>
#endif
#include <string>

#include "base/bind.h"
#include "base/files/file_path.h"
#include "base/message_loop/message_loop.h"
#include "base/native_library.h"
#include "base/stl_util.h"
#include "ipc/ipc_sender.h"
#include "ozone/platform/messages.h"
#include "ozone/wayland/data_device.h"
#include "ozone/wayland/display_poll_thread.h"
#include "ozone/wayland/egl/surface_ozone_wayland.h"
#if defined(ENABLE_DRM_SUPPORT)
#include "ozone/wayland/egl/wayland_pixmap.h"
#endif
#include "ozone/wayland/input/cursor.h"
#include "ozone/wayland/protocol/text-client-protocol.h"
#if defined(ENABLE_DRM_SUPPORT)
#include "ozone/wayland/protocol/wayland-drm-protocol.h"
#endif
#include "ozone/wayland/screen.h"
#include "ozone/wayland/seat.h"
#include "ozone/wayland/shell/shell.h"
#include "ozone/wayland/window.h"
#include "ui/ozone/public/native_pixmap.h"
#include "ui/ozone/public/surface_ozone_canvas.h"

#if defined(ENABLE_DRM_SUPPORT)
namespace {
using ozonewayland::WaylandDisplay;
// os-compatibility
static struct wl_drm* m_drm = 0;

void drm_handle_device(void* data, struct wl_drm*, const char* device) {
  WaylandDisplay* display = static_cast<WaylandDisplay*>(data);
  display->DrmHandleDevice(device);
}

void drm_handle_format(void* data, struct wl_drm*, uint32_t format) {
  WaylandDisplay* d = static_cast<WaylandDisplay*>(data);
  d->SetWLDrmFormat(format);
}

void drm_handle_authenticated(void* data, struct wl_drm*) {
  WaylandDisplay* d = static_cast<WaylandDisplay*>(data);
  d->DrmAuthenticated();
}

void drm_capabilities_(void* data, struct wl_drm*, uint32_t value) {
  struct WaylandDisplay* d = static_cast<WaylandDisplay*>(data);
  d->SetDrmCapabilities(value);
}

static const struct wl_drm_listener drm_listener = {
    drm_handle_device,
    drm_handle_format,
    drm_handle_authenticated
};

}  // namespace
#endif

namespace ozonewayland {
WaylandDisplay* WaylandDisplay::instance_ = NULL;

WaylandDisplay::WaylandDisplay() : SurfaceFactoryOzone(),
    display_(NULL),
    registry_(NULL),
    compositor_(NULL),
    data_device_manager_(NULL),
    shell_(NULL),
    shm_(NULL),
    text_input_manager_(NULL),
    primary_screen_(NULL),
    primary_seat_(NULL),
    display_poll_thread_(NULL),
    device_(NULL),
    m_deviceName(NULL),
    sender_(NULL),
    loop_(NULL),
    screen_list_(),
    seat_list_(),
    widget_map_(),
    serial_(0),
    processing_events_(false),
    m_authenticated_(false),
    m_fd_(-1),
    m_capabilities_(0),
    weak_ptr_factory_(this) {
}

WaylandDisplay::~WaylandDisplay() {
  Terminate();
}

const std::list<WaylandScreen*>& WaylandDisplay::GetScreenList() const {
  return screen_list_;
}

WaylandWindow* WaylandDisplay::GetWindow(unsigned window_handle) const {
  return GetWidget(window_handle);
}

struct wl_text_input_manager* WaylandDisplay::GetTextInputManager() const {
  return text_input_manager_;
}

void WaylandDisplay::FlushDisplay() {
  wl_display_flush(display_);
}

void WaylandDisplay::DestroyWindow(unsigned w) {
  std::map<unsigned, WaylandWindow*>::const_iterator it = widget_map_.find(w);
  WaylandWindow* widget = it == widget_map_.end() ? NULL : it->second;
  DCHECK(widget);
  delete widget;
  widget_map_.erase(w);
  if (widget_map_.empty())
    StopProcessingEvents();
}

gfx::AcceleratedWidget WaylandDisplay::GetNativeWindow(unsigned window_handle) {
  WaylandWindow* widget = GetWidget(window_handle);
  DCHECK(widget);
  widget->RealizeAcceleratedWidget();

  return (gfx::AcceleratedWidget)widget->egl_window();
}

bool WaylandDisplay::InitializeHardware() {
  InitializeDisplay();
  if (!display_) {
    LOG(ERROR) << "WaylandDisplay failed to initialize hardware";
    return false;
  }

  // Ensure we are processing wayland event requests. This needs to be done here
  // so we start polling before sandbox is initialized.
  StartProcessingEvents();
  return true;
}

intptr_t WaylandDisplay::GetNativeDisplay() {
  return (intptr_t)display_;
}

scoped_ptr<ui::SurfaceOzoneEGL> WaylandDisplay::CreateEGLSurfaceForWidget(
    gfx::AcceleratedWidget w) {
  return make_scoped_ptr<ui::SurfaceOzoneEGL>(new SurfaceOzoneWayland(w));
}

bool WaylandDisplay::LoadEGLGLES2Bindings(
    ui::SurfaceFactoryOzone::AddGLLibraryCallback add_gl_library,
    ui::SurfaceFactoryOzone::SetGLGetProcAddressProcCallback setprocaddress) {
  // The variable EGL_PLATFORM specifies native platform to be used by the
  // drivers (atleast on Mesa). When the variable is not set, Mesa uses the
  // first platform listed in --with-egl-platforms during compilation. Thus, we
  // ensure here that wayland is set as the native platform. However, we don't
  // override the EGL_PLATFORM value in case it has already been set.
  setenv("EGL_PLATFORM", "wayland", 0);
  base::NativeLibraryLoadError error;
  base::NativeLibrary gles_library = base::LoadNativeLibrary(
    base::FilePath("libGLESv2.so.2"), &error);

  if (!gles_library) {
    LOG(WARNING) << "Failed to load GLES library: " << error.ToString();
    return false;
  }

  base::NativeLibrary egl_library = base::LoadNativeLibrary(
    base::FilePath("libEGL.so.1"), &error);

  if (!egl_library) {
    LOG(WARNING) << "Failed to load EGL library: " << error.ToString();
    base::UnloadNativeLibrary(gles_library);
    return false;
  }

  GLGetProcAddressProc get_proc_address =
      reinterpret_cast<GLGetProcAddressProc>(
          base::GetFunctionPointerFromNativeLibrary(
              egl_library, "eglGetProcAddress"));

  if (!get_proc_address) {
    LOG(ERROR) << "eglGetProcAddress not found.";
    base::UnloadNativeLibrary(egl_library);
    base::UnloadNativeLibrary(gles_library);
    return false;
  }

  setprocaddress.Run(get_proc_address);
  add_gl_library.Run(egl_library);
  add_gl_library.Run(gles_library);
  return true;
}

const int32*
WaylandDisplay::GetEGLSurfaceProperties(const int32* desired_list) {
  static const EGLint kConfigAttribs[] = {
    EGL_BUFFER_SIZE, 32,
    EGL_ALPHA_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
  // According to egl spec depth size defaulted to zero and smallest size
  // preffered. Force depth to 24 bits to have same depth buffer on different
  // platforms.

    EGL_DEPTH_SIZE, 24,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_NONE
  };

  return kConfigAttribs;
}

scoped_refptr<ui::NativePixmap> WaylandDisplay::CreateNativePixmap(
    gfx::AcceleratedWidget widget,
    gfx::Size size,
    gfx::BufferFormat format,
    gfx::BufferUsage usage) {
#if defined(ENABLE_DRM_SUPPORT)
  if (usage == MAP)
    return NULL;

  scoped_refptr<WaylandPixmap> pixmap(new WaylandPixmap());
  if (!pixmap->Initialize(device_, format, size))
    return NULL;

  return pixmap;
#else
  return SurfaceFactoryOzone::CreateNativePixmap(widget, size, format, usage);
#endif
}

scoped_ptr<ui::SurfaceOzoneCanvas> WaylandDisplay::CreateCanvasForWidget(
    gfx::AcceleratedWidget widget) {
  LOG(FATAL) << "The browser process has attempted to start the GPU process in "
             << "software rendering mode. Software rendering is not supported "
             << "in Ozone-Wayland, so this is fatal. Usually this error occurs "
             << "because the GPU process crashed in hardware rendering mode, "
             << "often due to failure to initialize EGL. To debug the GPU "
             << "process, start Chrome with --gpu-startup-dialog so that the "
             << "GPU process pauses on startup, then attach to it with "
             << "'gdb -p' and run the command 'signal SIGUSR1' in order to "
             << "unpause it. If you have xterm then it is easier to run "
             << "'chrome --no-sandbox --gpu-launcher='xterm -title renderer "
             << "-e gdb --eval-command=run --args''";

  // This code will obviously never be reached, but it placates -Wreturn-type.
  return scoped_ptr<ui::SurfaceOzoneCanvas>();
}

void WaylandDisplay::InitializeDisplay() {
  DCHECK(!display_);
  display_ = wl_display_connect(NULL);
  if (!display_)
    return;

  instance_ = this;
  static const struct wl_registry_listener registry_all = {
    WaylandDisplay::DisplayHandleGlobal
  };

  registry_ = wl_display_get_registry(display_);
  wl_registry_add_listener(registry_, &registry_all, this);
  shell_ = new WaylandShell();

  if (wl_display_roundtrip(display_) < 0) {
    Terminate();
    return;
  }

  display_poll_thread_ = new WaylandDisplayPollThread(display_);
}

WaylandWindow* WaylandDisplay::CreateAcceleratedSurface(unsigned w) {
  WaylandWindow* window = new WaylandWindow(w);
  widget_map_[w] = window;

  return window;
}

void WaylandDisplay::StartProcessingEvents() {
  DCHECK(display_poll_thread_);
  // Start polling for wayland events.
  if (!processing_events_) {
    display_poll_thread_->StartProcessingEvents();
    processing_events_ = true;
  }
}

void WaylandDisplay::StopProcessingEvents() {
  DCHECK(display_poll_thread_);
  // Start polling for wayland events.
  if (processing_events_) {
    display_poll_thread_->StopProcessingEvents();
    processing_events_ = false;
  }
}

void WaylandDisplay::Terminate() {
  loop_ = NULL;
  if (!widget_map_.empty()) {
    STLDeleteValues(&widget_map_);
    widget_map_.clear();
  }

  for (WaylandSeat* seat : seat_list_)
    delete seat;

  for (WaylandScreen* screen : screen_list_)
    delete screen;

  screen_list_.clear();
  seat_list_.clear();

  if (text_input_manager_)
    wl_text_input_manager_destroy(text_input_manager_);

  if (data_device_manager_)
    wl_data_device_manager_destroy(data_device_manager_);

#if defined(ENABLE_DRM_SUPPORT)
  if (m_deviceName)
    delete m_deviceName;

  if (m_drm) {
    wl_drm_destroy(m_drm);
    m_drm = NULL;
  }

  close(m_fd_);
#endif
  if (compositor_)
    wl_compositor_destroy(compositor_);

  delete shell_;
  if (shm_)
    wl_shm_destroy(shm_);

  if (registry_)
    wl_registry_destroy(registry_);

  delete display_poll_thread_;

  if (display_) {
    wl_display_flush(display_);
    wl_display_disconnect(display_);
    display_ = NULL;
  }

  while (!deferred_messages_.empty())
    deferred_messages_.pop();

  instance_ = NULL;
}

WaylandWindow* WaylandDisplay::GetWidget(unsigned w) const {
  std::map<unsigned, WaylandWindow*>::const_iterator it = widget_map_.find(w);
  return it == widget_map_.end() ? NULL : it->second;
}

void WaylandDisplay::SetWidgetState(unsigned w, ui::WidgetState state) {
  switch (state) {
    case ui::FULLSCREEN:
    {
      WaylandWindow* widget = GetWidget(w);
      widget->SetFullscreen();
      break;
    }
    case ui::MAXIMIZED:
    {
      WaylandWindow* widget = GetWidget(w);
      widget->Maximize();
      break;
    }
    case ui::MINIMIZED:
    {
      WaylandWindow* widget = GetWidget(w);
      widget->Minimize();
      break;
    }
    case ui::RESTORE:
    {
      WaylandWindow* widget = GetWidget(w);
      widget->Restore();
      break;
    }
    case ui::ACTIVE:
      NOTIMPLEMENTED() << " ACTIVE " << w;
      break;
    case ui::INACTIVE:
      NOTIMPLEMENTED() << " INACTIVE " << w;
      break;
    case ui::SHOW:
    {
      WaylandWindow* widget = GetWidget(w);
      widget->Show();
      break;
    }
    case ui::HIDE:
    {
      WaylandWindow* widget = GetWidget(w);
      widget->Hide();
      break;
    }
    default:
      break;
  }
}

void WaylandDisplay::SetWidgetTitle(unsigned w, const base::string16& title) {
  WaylandWindow* widget = GetWidget(w);
  DCHECK(widget);
  widget->SetWindowTitle(title);
}

void WaylandDisplay::CreateWidget(unsigned widget,
                                  unsigned parent,
                                  int x,
                                  int y,
                                  ui::WidgetType type) {
  DCHECK(!GetWidget(widget));
  WaylandWindow* window = CreateAcceleratedSurface(widget);

  WaylandWindow* parent_window = GetWidget(parent);
  DCHECK(window);
  switch (type) {
  case ui::WINDOW:
    window->SetShellAttributes(WaylandWindow::TOPLEVEL);
    break;
  case ui::WINDOWFRAMELESS:
    NOTIMPLEMENTED();
    break;
  case ui::POPUP:
  case ui::TOOLTIP:
    DCHECK(parent_window);
    window->SetShellAttributes(WaylandWindow::POPUP,
                               parent_window->ShellSurface(),
                               x,
                               y);
    break;
  default:
    break;
  }
}

void WaylandDisplay::MoveWindow(unsigned widget,
                                unsigned parent,
                                ui::WidgetType type,
                                const gfx::Rect& rect) {
  WaylandWindow::ShellType shell_type;
  switch (type) {
  case ui::WINDOW:
    shell_type = WaylandWindow::TOPLEVEL;
    break;
  case ui::WINDOWFRAMELESS:
    NOTIMPLEMENTED();
    break;
  case ui::POPUP:
  case ui::TOOLTIP:
    shell_type = WaylandWindow::POPUP;
    break;
  default:
    break;
  }

  WaylandWindow* popup = GetWidget(widget);
  WaylandShellSurface* shell_parent  = NULL;
  WaylandWindow* parent_window  = NULL;
  if (parent) {
    parent_window = GetWidget(parent);
    shell_parent = parent_window->ShellSurface();
  }
  popup->Move(shell_type, shell_parent, rect);
}

void WaylandDisplay::AddRegion(unsigned handle, int left, int top,
                               int right, int bottom) {
  WaylandWindow* widget = GetWidget(handle);
  DCHECK(widget);
  widget->AddRegion(left, top, right, bottom);
}

void WaylandDisplay::SubRegion(unsigned handle, int left, int top,
                               int right, int bottom) {
  WaylandWindow* widget = GetWidget(handle);
  DCHECK(widget);
  widget->SubRegion(left, top, right, bottom);
}

void WaylandDisplay::SetCursorBitmap(const std::vector<SkBitmap>& bitmaps,
                                     const gfx::Point& location) {
  primary_seat_->SetCursorBitmap(bitmaps, location);
}

void WaylandDisplay::MoveCursor(const gfx::Point& location) {
  primary_seat_->MoveCursor(location);
}

void WaylandDisplay::ResetIme() {
  primary_seat_->ResetIme();
}

void WaylandDisplay::ImeCaretBoundsChanged(gfx::Rect rect) {
  primary_seat_->ImeCaretBoundsChanged(rect);
}

void WaylandDisplay::ShowInputPanel() {
  primary_seat_->ShowInputPanel();
}

void WaylandDisplay::HideInputPanel() {
  primary_seat_->HideInputPanel();
}

void WaylandDisplay::RequestDragData(const std::string& mime_type) {
  primary_seat_->GetDataDevice()->RequestDragData(mime_type);
}

void WaylandDisplay::RequestSelectionData(const std::string& mime_type) {
  primary_seat_->GetDataDevice()->RequestSelectionData(mime_type);
}

void WaylandDisplay::DragWillBeAccepted(uint32_t serial,
                                        const std::string& mime_type) {
  primary_seat_->GetDataDevice()->DragWillBeAccepted(serial, mime_type);
}

void WaylandDisplay::DragWillBeRejected(uint32_t serial) {
  primary_seat_->GetDataDevice()->DragWillBeRejected(serial);
}

#if defined(ENABLE_DRM_SUPPORT)
void WaylandDisplay::DrmHandleDevice(const char* device) {
  drm_magic_t magic;
  m_deviceName = strdup(device);

  if (!m_deviceName)
    return;
  int flags = O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC;
#ifdef O_CLOEXEC
  m_fd_ = open(device, flags, 0644);
  if (m_fd_ == -1 && errno == EINVAL) {
#endif
    m_fd_ = open(m_deviceName, flags, 0644);
    if (m_fd_ != -1)
      fcntl(m_fd_, F_SETFD, fcntl(m_fd_, F_GETFD) | FD_CLOEXEC);
#ifdef O_CLOEXEC
  }
#endif
  if (m_fd_ == -1) {
    LOG(ERROR) << "WaylandDisplay: could not open" << m_deviceName
        << strerror(errno);
    return;
  }

  drmGetMagic(m_fd_, &magic);
  wl_drm_authenticate(m_drm, magic);
}

void WaylandDisplay::SetWLDrmFormat(uint32_t) {
}

void WaylandDisplay::DrmAuthenticated() {
  m_authenticated_ = true;
  device_ = gbm_create_device(m_fd_);
  if (!device_) {
    LOG(ERROR) << "WaylandDisplay: Failed to create GBM Device.";
    close(m_fd_);
    m_fd_ = -1;
  }
}

void WaylandDisplay::SetDrmCapabilities(uint32_t value) {
  m_capabilities_ = value;
}
#endif
// static
void WaylandDisplay::DisplayHandleGlobal(void *data,
    struct wl_registry *registry,
    uint32_t name,
    const char *interface,
    uint32_t version) {

  WaylandDisplay* disp = static_cast<WaylandDisplay*>(data);

  if (strcmp(interface, "wl_compositor") == 0) {
    disp->compositor_ = static_cast<wl_compositor*>(
        wl_registry_bind(registry, name, &wl_compositor_interface, 1));
  } else if (strcmp(interface, "wl_data_device_manager") == 0) {
    disp->data_device_manager_ = static_cast<wl_data_device_manager*>(
        wl_registry_bind(registry, name, &wl_data_device_manager_interface, 1));
#if defined(ENABLE_DRM_SUPPORT)
  } else if (!strcmp(interface, "wl_drm")) {
    m_drm = static_cast<struct wl_drm*>(wl_registry_bind(registry,
                                                         name,
                                                         &wl_drm_interface,
                                                         1));
    wl_drm_add_listener(m_drm, &drm_listener, disp);
#endif
  } else if (strcmp(interface, "wl_output") == 0) {
    WaylandScreen* screen = new WaylandScreen(disp->registry(), name);
    if (!disp->screen_list_.empty())
      NOTIMPLEMENTED() << "Multiple screens support is not implemented";

    disp->screen_list_.push_back(screen);
    // (kalyan) Support extended output.
    disp->primary_screen_ = disp->screen_list_.front();
  } else if (strcmp(interface, "wl_seat") == 0) {
    // TODO(mcatanzaro): The display passed to WaylandInputDevice must have a
    // valid data device manager. We should ideally be robust to the compositor
    // advertising a wl_seat first. No known compositor does this, fortunately.
    CHECK(disp->data_device_manager_);
    WaylandSeat* seat = new WaylandSeat(disp, name);
    disp->seat_list_.push_back(seat);
    disp->primary_seat_ = disp->seat_list_.front();
  } else if (strcmp(interface, "wl_shm") == 0) {
    disp->shm_ = static_cast<wl_shm*>(
        wl_registry_bind(registry, name, &wl_shm_interface, 1));
  } else if (strcmp(interface, "wl_text_input_manager") == 0) {
    disp->text_input_manager_ = static_cast<wl_text_input_manager*>(
        wl_registry_bind(registry, name, &wl_text_input_manager_interface, 1));
  } else {
    disp->shell_->Initialize(registry, name, interface, version);
  }
}

void WaylandDisplay::OnChannelEstablished(IPC::Sender* sender) {
  loop_ = base::MessageLoop::current();
  sender_ = sender;
  while (!deferred_messages_.empty()) {
    Dispatch(deferred_messages_.front());
    deferred_messages_.pop();
  }
}

bool WaylandDisplay::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(WaylandDisplay, message)
  IPC_MESSAGE_HANDLER(WaylandDisplay_State, SetWidgetState)
  IPC_MESSAGE_HANDLER(WaylandDisplay_Create, CreateWidget)
  IPC_MESSAGE_HANDLER(WaylandDisplay_MoveWindow, MoveWindow)
  IPC_MESSAGE_HANDLER(WaylandDisplay_Title, SetWidgetTitle)
  IPC_MESSAGE_HANDLER(WaylandDisplay_AddRegion, AddRegion)
  IPC_MESSAGE_HANDLER(WaylandDisplay_SubRegion, SubRegion)
  IPC_MESSAGE_HANDLER(WaylandDisplay_CursorSet, SetCursorBitmap)
  IPC_MESSAGE_HANDLER(WaylandDisplay_MoveCursor, MoveCursor)
  IPC_MESSAGE_HANDLER(WaylandDisplay_ImeReset, ResetIme)
  IPC_MESSAGE_HANDLER(WaylandDisplay_ShowInputPanel, ShowInputPanel)
  IPC_MESSAGE_HANDLER(WaylandDisplay_HideInputPanel, HideInputPanel)
  IPC_MESSAGE_HANDLER(WaylandDisplay_RequestDragData, RequestDragData)
  IPC_MESSAGE_HANDLER(WaylandDisplay_RequestSelectionData, RequestSelectionData)
  IPC_MESSAGE_HANDLER(WaylandDisplay_DragWillBeAccepted, DragWillBeAccepted)
  IPC_MESSAGE_HANDLER(WaylandDisplay_DragWillBeRejected, DragWillBeRejected)
  IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

IPC::MessageFilter* WaylandDisplay::GetMessageFilter() {
  return NULL;
}

void WaylandDisplay::MotionNotify(float x, float y) {
  Dispatch(new WaylandInput_MotionNotify(x, y));
}

void WaylandDisplay::ButtonNotify(unsigned handle,
                                  ui::EventType type,
                                  ui::EventFlags flags,
                                  float x,
                                  float y) {
  Dispatch(new WaylandInput_ButtonNotify(handle, type, flags, x, y));
}

void WaylandDisplay::AxisNotify(float x,
                                float y,
                                int xoffset,
                                int yoffset) {
  Dispatch(new WaylandInput_AxisNotify(x, y, xoffset, yoffset));
}

void WaylandDisplay::PointerEnter(unsigned handle, float x, float y) {
  Dispatch(new WaylandInput_PointerEnter(handle, x, y));
}

void WaylandDisplay::PointerLeave(unsigned handle, float x, float y) {
  Dispatch(new WaylandInput_PointerLeave(handle, x, y));
}

void WaylandDisplay::KeyNotify(ui::EventType type,
                               unsigned code,
                               int device_id) {
  Dispatch(new WaylandInput_KeyNotify(type, code, device_id));
}

void WaylandDisplay::VirtualKeyNotify(ui::EventType type,
                                      uint32_t key,
                                      int device_id) {
  Dispatch(new WaylandInput_VirtualKeyNotify(type, key, device_id));
}

void WaylandDisplay::TouchNotify(ui::EventType type,
                                 float x,
                                 float y,
                                 int32_t touch_id,
                                 uint32_t time_stamp) {
  Dispatch(new WaylandInput_TouchNotify(type, x, y, touch_id, time_stamp));
}

void WaylandDisplay::OutputSizeChanged(unsigned width, unsigned height) {
  Dispatch(new WaylandInput_OutputSize(width, height));
}

void WaylandDisplay::WindowResized(unsigned handle,
                                   unsigned width,
                                   unsigned height) {
  Dispatch(new WaylandWindow_Resized(handle, width, height));
}

void WaylandDisplay::WindowUnminimized(unsigned handle) {
  Dispatch(new WaylandWindow_Unminimized(handle));
}

void WaylandDisplay::WindowDeActivated(unsigned windowhandle) {
  Dispatch(new WaylandWindow_DeActivated(windowhandle));
}

void WaylandDisplay::WindowActivated(unsigned windowhandle) {
  Dispatch(new WaylandWindow_Activated(windowhandle));
}

void WaylandDisplay::CloseWidget(unsigned handle) {
  Dispatch(new WaylandInput_CloseWidget(handle));
}

void WaylandDisplay::Commit(unsigned handle,
                            const std::string& text) {
  Dispatch(new WaylandInput_Commit(handle, text));
}

void WaylandDisplay::PreeditChanged(unsigned handle,
                                    const std::string& text,
                                    const std::string& commit) {
  Dispatch(new WaylandInput_PreeditChanged(handle, text, commit));
}

void WaylandDisplay::PreeditEnd() {
  Dispatch(new WaylandInput_PreeditEnd());
}

void WaylandDisplay::PreeditStart() {
  Dispatch(new WaylandInput_PreeditStart());
}

void WaylandDisplay::InitializeXKB(base::SharedMemoryHandle fd, uint32_t size) {
  Dispatch(new WaylandInput_InitializeXKB(fd, size));
}

void WaylandDisplay::DragEnter(unsigned windowhandle,
                               float x,
                               float y,
                               const std::vector<std::string>& mime_types,
                               uint32_t serial) {
  Dispatch(new WaylandInput_DragEnter(windowhandle, x, y, mime_types, serial));
}

void WaylandDisplay::DragData(unsigned windowhandle,
                              base::FileDescriptor pipefd) {
  Dispatch(new WaylandInput_DragData(windowhandle, pipefd));
}

void WaylandDisplay::DragLeave(unsigned windowhandle) {
  Dispatch(new WaylandInput_DragLeave(windowhandle));
}

void WaylandDisplay::DragMotion(unsigned windowhandle,
                                float x,
                                float y,
                                uint32_t time) {
  Dispatch(new WaylandInput_DragMotion(windowhandle, x, y, time));
}

void WaylandDisplay::DragDrop(unsigned windowhandle) {
  Dispatch(new WaylandInput_DragDrop(windowhandle));
}

void WaylandDisplay::Dispatch(IPC::Message* message) {
  if (!loop_) {
    deferred_messages_.push(message);
    return;
  }

  loop_->task_runner()->PostTask(FROM_HERE,
      base::Bind(&WaylandDisplay::Send,
                 weak_ptr_factory_.GetWeakPtr(),
                 message));
}

void WaylandDisplay::Send(IPC::Message* message) {
  // The GPU process never sends synchronous IPC, so clear the unblock flag.
  // This ensures the message is treated as a synchronous one and helps preserve
  // order. Check set_unblock in ipc_messages.h for explanation.
  message->set_unblock(true);
  sender_->Send(message);
}

}  // namespace ozonewayland

