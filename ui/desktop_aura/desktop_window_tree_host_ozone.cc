// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/desktop_aura/desktop_window_tree_host_ozone.h"

#include <string>

#include "base/bind.h"
#include "ozone/ui/desktop_aura/desktop_drag_drop_client_wayland.h"
#include "ozone/ui/desktop_aura/desktop_screen_wayland.h"
#include "ui/aura/client/focus_client.h"
#include "ui/aura/window_property.h"
#include "ui/base/hit_test.h"
#include "ui/base/ime/input_method.h"
#include "ui/events/platform/platform_event_source.h"
#include "ui/gfx/geometry/insets.h"
#include "ui/gfx/path.h"
#include "ui/native_theme/native_theme.h"
#include "ui/ozone/public/ozone_platform.h"
#include "ui/platform_window/platform_window.h"
#include "ui/views/corewm/tooltip_aura.h"
#include "ui/views/linux_ui/linux_ui.h"
#include "ui/views/views_delegate.h"
#include "ui/views/views_export.h"
#include "ui/views/widget/desktop_aura/desktop_dispatcher_client.h"
#include "ui/views/widget/desktop_aura/desktop_native_cursor_manager.h"
#include "ui/views/widget/desktop_aura/desktop_native_widget_aura.h"
#include "ui/views/widget/desktop_aura/desktop_screen_position_client.h"
#include "ui/wm/core/window_util.h"
#include "ui/wm/public/window_move_client.h"

DECLARE_WINDOW_PROPERTY_TYPE(views::DesktopWindowTreeHostOzone*);

namespace views {

std::list<gfx::AcceleratedWidget>*
DesktopWindowTreeHostOzone::open_windows_ = NULL;

std::vector<aura::Window*>*
DesktopWindowTreeHostOzone::aura_windows_ = NULL;

DEFINE_WINDOW_PROPERTY_KEY(
    aura::Window*, kViewsWindowForRootWindow, NULL);

DEFINE_WINDOW_PROPERTY_KEY(
    DesktopWindowTreeHostOzone*, kHostForRootWindow, NULL);

DesktopWindowTreeHostOzone::DesktopWindowTreeHostOzone(
    internal::NativeWidgetDelegate* native_widget_delegate,
    DesktopNativeWidgetAura* desktop_native_widget_aura)
    : aura::WindowTreeHost(),
      state_(Uninitialized),
      has_capture_(false),
      custom_window_shape_(false),
      always_on_top_(false),
      previous_bounds_(0, 0, 0, 0),
      previous_maximize_bounds_(0, 0, 0, 0),
      window_(0),
      title_(base::string16()),
      drag_drop_client_(NULL),
      native_widget_delegate_(native_widget_delegate),
      content_window_(NULL),
      desktop_native_widget_aura_(desktop_native_widget_aura),
      window_parent_(NULL),
      window_children_(),
      close_widget_factory_(this) {
}

DesktopWindowTreeHostOzone::~DesktopWindowTreeHostOzone() {
  window()->ClearProperty(kHostForRootWindow);
  aura::client::SetWindowMoveClient(window(), NULL);
  desktop_native_widget_aura_->OnDesktopWindowTreeHostDestroyed(this);
  DestroyDispatcher();
}

// static
aura::Window*
DesktopWindowTreeHostOzone::GetContentWindowForAcceleratedWidget(
    gfx::AcceleratedWidget widget) {
  aura::WindowTreeHost* host =
      aura::WindowTreeHost::GetForAcceleratedWidget(widget);

  return host ?
      host->window()->GetProperty(kViewsWindowForRootWindow) : NULL;
}

// static
DesktopWindowTreeHostOzone*
DesktopWindowTreeHostOzone::GetHostForAcceleratedWidget(
    gfx::AcceleratedWidget widget) {
  aura::WindowTreeHost* host =
      aura::WindowTreeHost::GetForAcceleratedWidget(widget);

  return host ? host->window()->GetProperty(kHostForRootWindow) : NULL;
}

// static
const std::vector<aura::Window*>&
DesktopWindowTreeHostOzone::GetAllOpenWindows() {
  if (!aura_windows_) {
    const std::list<gfx::AcceleratedWidget>& windows = open_windows();
    aura_windows_ = new std::vector<aura::Window*>(windows.size());
    std::transform(
        windows.begin(), windows.end(), aura_windows_->begin(),
            DesktopWindowTreeHostOzone::GetContentWindowForAcceleratedWidget);
    }

  return *aura_windows_;
}

void DesktopWindowTreeHostOzone::CleanUpWindowList() {
  delete open_windows_;
  open_windows_ = NULL;
  if (aura_windows_) {
    aura_windows_->clear();
    delete aura_windows_;
    aura_windows_ = NULL;
  }
}

gfx::Rect DesktopWindowTreeHostOzone::GetBoundsInScreen() const {
  return platform_window_->GetBounds();
}

////////////////////////////////////////////////////////////////////////////////
// DesktopWindowTreeHostOzone, DesktopWindowTreeHost implementation:

void DesktopWindowTreeHostOzone::Init(
    aura::Window* content_window,
    const Widget::InitParams& params) {
  content_window_ = content_window;
  // In some situations, views tries to make a zero sized window, and that
  // makes us crash. Make sure we have valid sizes.
  Widget::InitParams sanitized_params = params;
  if (sanitized_params.bounds.width() == 0)
    sanitized_params.bounds.set_width(100);
  if (sanitized_params.bounds.height() == 0)
    sanitized_params.bounds.set_height(100);

  InitOzoneWindow(sanitized_params);
}

void DesktopWindowTreeHostOzone::OnNativeWidgetCreated(
    const Widget::InitParams& params) {
  window()->SetProperty(kViewsWindowForRootWindow, content_window_);
  window()->SetProperty(kHostForRootWindow, this);

  // If we're given a parent, we need to mark ourselves as transient to another
  // window. Otherwise activation gets screwy.
  gfx::NativeView parent = params.parent;
  if (!params.child && params.parent)
    wm::AddTransientChild(parent, content_window_);

  native_widget_delegate_->OnNativeWidgetCreated(true);
  open_windows().push_back(window_);
  if (aura_windows_) {
    aura_windows_->clear();
    delete aura_windows_;
    aura_windows_ = NULL;
  }
}

scoped_ptr<corewm::Tooltip>
DesktopWindowTreeHostOzone::CreateTooltip() {
  return scoped_ptr<views::corewm::Tooltip>(new views::corewm::TooltipAura);
}

scoped_ptr<aura::client::DragDropClient>
DesktopWindowTreeHostOzone::CreateDragDropClient(
    DesktopNativeCursorManager* cursor_manager) {
  drag_drop_client_ = new DesktopDragDropClientWayland(window(),
                                                       platform_window_.get());
  return make_scoped_ptr(drag_drop_client_);
}

void DesktopWindowTreeHostOzone::Close() {
  if (!close_widget_factory_.HasWeakPtrs()) {
    // And we delay the close so that if we are called from an ATL callback,
    // we don't destroy the window before the callback returned (as the caller
    // may delete ourselves on destroy and the ATL callback would still
    // dereference us when the callback returns).
    base::MessageLoop::current()->PostTask(
        FROM_HERE,
        base::Bind(&DesktopWindowTreeHostOzone::CloseNow,
                   close_widget_factory_.GetWeakPtr()));
  }
}

void DesktopWindowTreeHostOzone::CloseNow() {
  if (!window_)
    return;

  unsigned widgetId = window_;
  ReleaseCapture();
  native_widget_delegate_->OnNativeWidgetDestroying();

  // If we have children, close them. Use a copy for iteration because they'll
  // remove themselves.
  std::set<DesktopWindowTreeHostOzone*> window_children_copy =
      window_children_;
  for (std::set<DesktopWindowTreeHostOzone*>::iterator it =
           window_children_copy.begin(); it != window_children_copy.end();
       ++it) {
    (*it)->CloseNow();
  }
  DCHECK(window_children_.empty());

  // If we have a parent, remove ourselves from its children list.
  if (window_parent_) {
    window_parent_->window_children_.erase(this);
    window_parent_ = NULL;
  }

  // Destroy the compositor before destroying the window since shutdown
  // may try to swap, and the swap without a window causes an error, which
  // causes a crash with in-process renderer.
  DestroyCompositor();

  open_windows().remove(widgetId);
  if (aura_windows_) {
    aura_windows_->clear();
    delete aura_windows_;
    aura_windows_ = NULL;
  }

  // Actually free our native resources.
  platform_window_->Close();
  window_ = 0;
  if (open_windows().empty())
    CleanUpWindowList();

  desktop_native_widget_aura_->OnHostClosed();
}

aura::WindowTreeHost* DesktopWindowTreeHostOzone::AsWindowTreeHost() {
  return this;
}

void DesktopWindowTreeHostOzone::ShowWindowWithState(
    ui::WindowShowState show_state) {
  if (compositor())
    compositor()->SetVisible(true);
  state_ |= Visible;

  switch (show_state) {
    case ui::SHOW_STATE_NORMAL:
      Activate();
      break;
    case ui::SHOW_STATE_MAXIMIZED:
      Maximize();
      break;
    case ui::SHOW_STATE_MINIMIZED:
      Minimize();
      break;
    case ui::SHOW_STATE_FULLSCREEN:
      SetFullscreen(true);
      break;
    default:
      break;
  }

  native_widget_delegate_->AsWidget()->SetInitialFocus(show_state);
}

void DesktopWindowTreeHostOzone::ShowMaximizedWithBounds(
    const gfx::Rect& restored_bounds) {
  ShowWindowWithState(ui::SHOW_STATE_MAXIMIZED);
  previous_bounds_ = restored_bounds;
}

bool DesktopWindowTreeHostOzone::IsVisible() const {
  return state_ & Visible;
}

void DesktopWindowTreeHostOzone::SetSize(const gfx::Size& requested_size) {
  gfx::Size size_in_pixels = ToPixelRect(gfx::Rect(requested_size)).size();
  size_in_pixels = AdjustSize(size_in_pixels);
  gfx::Rect new_bounds = platform_window_->GetBounds();
  new_bounds.set_size(size_in_pixels);
  platform_window_->SetBounds(new_bounds);
}

void DesktopWindowTreeHostOzone::StackAbove(aura::Window* window) {
}

void DesktopWindowTreeHostOzone::StackAtTop() {
}

void DesktopWindowTreeHostOzone::CenterWindow(const gfx::Size& size) {
  gfx::Size size_in_pixels = ToPixelRect(gfx::Rect(size)).size();
  gfx::Rect parent_bounds_in_pixels = GetWorkAreaBoundsInScreen();

  // If |window_|'s transient parent bounds are big enough to contain |size|,
  // use them instead.
  if (wm::GetTransientParent(content_window_)) {
    gfx::Rect transient_parent_rect =
        wm::GetTransientParent(content_window_)->GetBoundsInScreen();
    if (transient_parent_rect.height() >= size.height() &&
      transient_parent_rect.width() >= size.width()) {
      parent_bounds_in_pixels = ToPixelRect(transient_parent_rect);
    }
  }

  gfx::Rect window_bounds_in_pixels(
      parent_bounds_in_pixels.x() +
          (parent_bounds_in_pixels.width() - size_in_pixels.width()) / 2,
      parent_bounds_in_pixels.y() +
          (parent_bounds_in_pixels.height() - size_in_pixels.height()) / 2,
      size_in_pixels.width(), size_in_pixels.height());
  // Don't size the window bigger than the parent, otherwise the user may not be
  // able to close or move it.
  window_bounds_in_pixels.AdjustToFit(parent_bounds_in_pixels);

  SetBounds(window_bounds_in_pixels);
}

void DesktopWindowTreeHostOzone::GetWindowPlacement(
    gfx::Rect* bounds,
    ui::WindowShowState* show_state) const {
  *bounds = GetRestoredBounds();

  if (IsMinimized()) {
    *show_state = ui::SHOW_STATE_MINIMIZED;
  } else if (IsFullscreen()) {
    *show_state = ui::SHOW_STATE_FULLSCREEN;
  } else if (IsMaximized()) {
    *show_state = ui::SHOW_STATE_MAXIMIZED;
  } else if (!IsActive()) {
    *show_state = ui::SHOW_STATE_INACTIVE;
  } else {
    *show_state = ui::SHOW_STATE_NORMAL;
  }
}

gfx::Rect DesktopWindowTreeHostOzone::GetWindowBoundsInScreen() const {
  return platform_window_->GetBounds();
}

gfx::Rect DesktopWindowTreeHostOzone::GetClientAreaBoundsInScreen() const {
  // TODO(erg): The NativeWidgetAura version returns |bounds_|, claiming its
  // needed for View::ConvertPointToScreen() to work
  // correctly. DesktopWindowTreeHostWin::GetClientAreaBoundsInScreen() just
  // asks windows what it thinks the client rect is.
  //
  // Attempts to calculate the rect by asking the NonClientFrameView what it
  // thought its GetBoundsForClientView() were broke combobox drop down
  // placement.
  return platform_window_->GetBounds();
}

gfx::Rect DesktopWindowTreeHostOzone::GetRestoredBounds() const {
  if (!previous_bounds_.IsEmpty())
    return ToDIPRect(previous_bounds_);

  return GetWindowBoundsInScreen();
}

gfx::Rect DesktopWindowTreeHostOzone::GetWorkAreaBoundsInScreen() const {
  // TODO(kalyan): Take into account wm decorations. i.e Dock, panel etc.
  gfx::Screen *screen = gfx::Screen::GetScreenByType(gfx::SCREEN_TYPE_NATIVE);
  if (!screen)
    NOTREACHED() << "Unable to retrieve valid gfx::Screen";

  gfx::Display display = screen->GetPrimaryDisplay();
  return ToDIPRect(display.bounds());
}

void DesktopWindowTreeHostOzone::SetShape(SkRegion* native_region) {
  custom_window_shape_ = false;
  gfx::Path window_mask;

  if (native_region) {
    gfx::Transform transform = GetRootTransform();
    if (!transform.IsIdentity() && !native_region->isEmpty()) {
      SkPath path_in_dip;
      if (native_region->getBoundaryPath(&path_in_dip)) {
        path_in_dip.transform(transform.matrix(), &window_mask);
      }
    }

    custom_window_shape_ = true;
    delete native_region;
  }

  platform_window_->SetWindowShape(window_mask);
}

void DesktopWindowTreeHostOzone::Activate() {
  if (state_ & Visible) {
    OnActivationChanged(true);
  }
}

void DesktopWindowTreeHostOzone::Deactivate() {
  OnActivationChanged(false);
}

bool DesktopWindowTreeHostOzone::IsActive() const {
  return state_ & Active;
}

void DesktopWindowTreeHostOzone::Maximize() {
  if (state_ & Maximized)
    return;

  state_ |= Maximized;
  state_ &= ~Minimized;
  previous_bounds_ = platform_window_->GetBounds();
  platform_window_->Maximize();
  if (IsMinimized())
    ShowWindowWithState(ui::SHOW_STATE_MAXIMIZED);
}

void DesktopWindowTreeHostOzone::Minimize() {
  if (state_ & Minimized)
    return;

  state_ |= Minimized;
  previous_bounds_ = platform_window_->GetBounds();
  ReleaseCapture();
  compositor()->SetVisible(false);
  content_window_->Hide();
  platform_window_->Minimize();
  Relayout();
}

void DesktopWindowTreeHostOzone::Restore() {
  state_ &= ~Maximized;
  if (state_ & Minimized) {
    content_window_->Show();
    compositor()->SetVisible(true);
  }

  platform_window_->Restore();
  platform_window_->SetBounds(previous_bounds_);
  previous_bounds_ = gfx::Rect();
  Relayout();
  if (state_ & Minimized) {
    state_ &= ~Minimized;
    ShowWindow();
  }
}

bool DesktopWindowTreeHostOzone::IsMaximized() const {
  return !IsFullscreen() && (state_ & Maximized);
}

bool DesktopWindowTreeHostOzone::IsMinimized() const {
  return state_ & Minimized;
}

bool DesktopWindowTreeHostOzone::HasCapture() const {
  return has_capture_;
}

void DesktopWindowTreeHostOzone::SetAlwaysOnTop(bool always_on_top) {
  always_on_top_ = always_on_top;
}

bool DesktopWindowTreeHostOzone::IsAlwaysOnTop() const {
  return always_on_top_;
}

void DesktopWindowTreeHostOzone::SetVisibleOnAllWorkspaces(
    bool always_visible) {
  NOTIMPLEMENTED();
}

bool DesktopWindowTreeHostOzone::SetWindowTitle(const base::string16& title) {
  if (title.compare(title_)) {
    platform_window_->SetTitle(title);
    title_ = title;
    return true;
  }

  return false;
}

void DesktopWindowTreeHostOzone::ClearNativeFocus() {
  // This method is weird and misnamed. Instead of clearing the native focus,
  // it sets the focus to our |content_window_|, which will trigger a cascade
  // of focus changes into views.
  if (content_window_ && aura::client::GetFocusClient(content_window_) &&
      content_window_->Contains(
          aura::client::GetFocusClient(content_window_)->GetFocusedWindow())) {
    aura::client::GetFocusClient(content_window_)->FocusWindow(content_window_);
  }
}

Widget::MoveLoopResult DesktopWindowTreeHostOzone::RunMoveLoop(
    const gfx::Vector2d& drag_offset,
    Widget::MoveLoopSource source,
    Widget::MoveLoopEscapeBehavior escape_behavior) {
  NOTIMPLEMENTED();
  return Widget::MOVE_LOOP_SUCCESSFUL;
}

void DesktopWindowTreeHostOzone::EndMoveLoop() {
  NOTIMPLEMENTED();
}

void DesktopWindowTreeHostOzone::SetVisibilityChangedAnimationsEnabled(
    bool value) {
  // Much like the previous NativeWidgetGtk, we don't have anything to do here.
}

bool DesktopWindowTreeHostOzone::ShouldUseNativeFrame() const {
  return false;
}

bool DesktopWindowTreeHostOzone::ShouldWindowContentsBeTransparent() const {
  return false;
}

void DesktopWindowTreeHostOzone::FrameTypeChanged() {
  Widget::FrameType new_type =
    native_widget_delegate_->AsWidget()->frame_type();
  if (new_type == Widget::FRAME_TYPE_DEFAULT) {
    // The default is determined by Widget::InitParams::remove_standard_frame
    // and does not change.
    return;
  }

  // Replace the frame and layout the contents. Even though we don't have a
  // swapable glass frame like on Windows, we still replace the frame because
  // the button assets don't update otherwise.
  native_widget_delegate_->AsWidget()->non_client_view()->UpdateFrame();
}

void DesktopWindowTreeHostOzone::SetFullscreen(bool fullscreen) {
  if ((state_ & FullScreen) == fullscreen)
    return;

  if (fullscreen)
    state_ |= FullScreen;
  else
    state_ &= ~FullScreen;

  if (!(state_ & FullScreen)) {
    if (state_ & Maximized) {
      previous_bounds_ = previous_maximize_bounds_;
      previous_maximize_bounds_ = gfx::Rect();
      platform_window_->Maximize();
    } else {
      Restore();
    }
  } else {
    if (state_ & Maximized)
      previous_maximize_bounds_ = previous_bounds_;

    previous_bounds_ = platform_window_->GetBounds();
    platform_window_->ToggleFullscreen();
  }

  Relayout();
}

bool DesktopWindowTreeHostOzone::IsFullscreen() const {
  return state_ & FullScreen;
}

void DesktopWindowTreeHostOzone::SetOpacity(unsigned char opacity) {
  content_window_->layer()->SetOpacity(opacity / 255.0);
  platform_window_->SetOpacity(opacity);
}

void DesktopWindowTreeHostOzone::SetWindowIcons(
    const gfx::ImageSkia& window_icon, const gfx::ImageSkia& app_icon) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

void DesktopWindowTreeHostOzone::InitModalType(ui::ModalType modal_type) {
  switch (modal_type) {
    case ui::MODAL_TYPE_NONE:
      break;
    default:
      // TODO(erg): Figure out under what situations |modal_type| isn't
      // none. The comment in desktop_native_widget_aura.cc suggests that this
      // is rare.
      NOTIMPLEMENTED();
  }
}

void DesktopWindowTreeHostOzone::FlashFrame(bool flash_frame) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

void DesktopWindowTreeHostOzone::OnRootViewLayout() {
}

void DesktopWindowTreeHostOzone::OnNativeWidgetFocus() {
}

void DesktopWindowTreeHostOzone::OnNativeWidgetBlur() {
}

bool DesktopWindowTreeHostOzone::IsAnimatingClosed() const {
  return false;
}

bool DesktopWindowTreeHostOzone::IsTranslucentWindowOpacitySupported() const {
  return false;
}

void DesktopWindowTreeHostOzone::SizeConstraintsChanged() {
}


////////////////////////////////////////////////////////////////////////////////
// DesktopWindowTreeHostOzone, aura::WindowTreeHost implementation:

gfx::Transform DesktopWindowTreeHostOzone::GetRootTransform() const {
  gfx::Display display = gfx::Screen::GetNativeScreen()->GetPrimaryDisplay();
  aura::Window* win = const_cast<aura::Window*>(window());
  display = gfx::Screen::GetNativeScreen()->GetDisplayNearestWindow(win);

  float scale = display.device_scale_factor();
  gfx::Transform transform;
  transform.Scale(scale, scale);
  return transform;
}

ui::EventSource* DesktopWindowTreeHostOzone::GetEventSource() {
  return this;
}

gfx::AcceleratedWidget DesktopWindowTreeHostOzone::GetAcceleratedWidget() {
  return window_;
}

void DesktopWindowTreeHostOzone::ShowImpl() {
  if (state_ & Visible)
    return;

  ShowWindowWithState(ui::SHOW_STATE_NORMAL);
  native_widget_delegate_->OnNativeWidgetVisibilityChanged(true);
}

void DesktopWindowTreeHostOzone::HideImpl() {
  if (!(state_ & Visible))
    return;

  state_ &= ~Visible;
  platform_window_->Hide();
  native_widget_delegate_->OnNativeWidgetVisibilityChanged(false);
}

gfx::Rect DesktopWindowTreeHostOzone::GetBounds() const {
  return platform_window_->GetBounds();
}

void DesktopWindowTreeHostOzone::SetBounds(
    const gfx::Rect& requested_bounds) {
  gfx::Rect bounds(requested_bounds.origin(),
                   AdjustSize(requested_bounds.size()));
  platform_window_->SetBounds(bounds);
}

gfx::Point DesktopWindowTreeHostOzone::GetLocationOnNativeScreen() const {
  return platform_window_->GetBounds().origin();
}

void DesktopWindowTreeHostOzone::SetCapture() {
  if (has_capture_)
    return;

  has_capture_ = true;
  platform_window_->SetCapture();
}

void DesktopWindowTreeHostOzone::ReleaseCapture() {
  platform_window_->ReleaseCapture();
  OnLostCapture();
}

void DesktopWindowTreeHostOzone::ShowWindow() {
  ui::WindowShowState show_state = ui::SHOW_STATE_NORMAL;
  if (IsMinimized()) {
    show_state = ui::SHOW_STATE_MINIMIZED;
  } else if (IsFullscreen()) {
    show_state = ui::SHOW_STATE_FULLSCREEN;
  } else if (IsMaximized()) {
    show_state = ui::SHOW_STATE_MAXIMIZED;
  } else if (!IsActive()) {
    show_state = ui::SHOW_STATE_INACTIVE;
  }

  ShowWindowWithState(show_state);
}

void DesktopWindowTreeHostOzone::SetCursorNative(gfx::NativeCursor cursor) {
  platform_window_->SetCursor(cursor.platform());
}

void DesktopWindowTreeHostOzone::MoveCursorToNative(
    const gfx::Point& location) {
  platform_window_->MoveCursorTo(location);
}

void DesktopWindowTreeHostOzone::OnCursorVisibilityChangedNative(bool show) {
  // TODO(erg): Conditional on us enabling touch on desktop linux builds, do
  // the same tap-to-click disabling here that chromeos does.
}

////////////////////////////////////////////////////////////////////////////////
// ui::PlatformWindowDelegate implementation:
void DesktopWindowTreeHostOzone::OnBoundsChanged(
    const gfx::Rect& new_bounds) {
  // TODO(kalyan): Add support to check if origin has really changed.
  native_widget_delegate_->AsWidget()->OnNativeWidgetMove();
  OnHostResized(new_bounds.size());
  ResetWindowRegion();
}

void DesktopWindowTreeHostOzone::OnDamageRect(const gfx::Rect& damaged_rect) {
  compositor()->ScheduleRedrawRect(damaged_rect);
}

void DesktopWindowTreeHostOzone::OnAcceleratedWidgetDestroyed() {
  gfx::AcceleratedWidget window = compositor()->ReleaseAcceleratedWidget();
  DCHECK_EQ(window, window_);
  window_ = gfx::kNullAcceleratedWidget;
}

void DesktopWindowTreeHostOzone::OnActivationChanged(bool active) {
  if (active == (state_ & Active))
    return;

  if (active) {
    // Make sure the stacking order is correct. The activated window should be
    // first one in list of open windows.
    std::list<gfx::AcceleratedWidget>& windows = open_windows();
    DCHECK(windows.size());
    if (windows.front() != window_) {
      windows.remove(window_);
      windows.insert(windows.begin(), window_);
    }

    state_ |= Active;
    OnHostActivated();
  } else {
    state_ &= ~Active;
    ReleaseCapture();
  }

  desktop_native_widget_aura_->HandleActivationChanged(active);
  native_widget_delegate_->AsWidget()->GetRootView()->SchedulePaint();
}

void DesktopWindowTreeHostOzone::OnDragEnter(
    unsigned windowhandle,
    float x,
    float y,
    const std::vector<std::string>& mime_types,
    uint32_t serial) {
  if (drag_drop_client_)
    drag_drop_client_->OnDragEnter(windowhandle, x, y, mime_types, serial);
}

void DesktopWindowTreeHostOzone::OnDragDataReceived(int fd) {
  if (drag_drop_client_)
    drag_drop_client_->OnDragDataReceived(fd);
  else
    close(fd);
}

void DesktopWindowTreeHostOzone::OnDragLeave() {
  if (drag_drop_client_)
    drag_drop_client_->OnDragLeave();
}

void DesktopWindowTreeHostOzone::OnDragMotion(float x,
                                              float y,
                                              uint32_t time) {
  if (drag_drop_client_)
    drag_drop_client_->OnDragMotion(x, y, time);
}

void DesktopWindowTreeHostOzone::OnDragDrop() {
  if (drag_drop_client_)
    drag_drop_client_->OnDragDrop();
}

void DesktopWindowTreeHostOzone::OnLostCapture() {
  OnHostLostWindowCapture();
  has_capture_ = false;
}

void DesktopWindowTreeHostOzone::OnAcceleratedWidgetAvailable(
      gfx::AcceleratedWidget widget,
       float device_pixel_ratio) {
  window_ = widget;
  CreateCompositor();
  WindowTreeHost::OnAcceleratedWidgetAvailable();
}

void DesktopWindowTreeHostOzone::OnCloseRequest() {
  Close();
}

void DesktopWindowTreeHostOzone::OnClosed() {
  CloseNow();
}

void DesktopWindowTreeHostOzone::OnWindowStateChanged(
    ui::PlatformWindowState new_state) {
  switch (new_state) {
    case ui::PLATFORM_WINDOW_STATE_MAXIMIZED: {
      if (state_ & Minimized) {
        content_window_->Show();
        compositor()->SetVisible(true);
        state_ &= ~Minimized;
      }
      platform_window_->SetBounds(previous_bounds_);
      previous_bounds_ = gfx::Rect();
      Relayout();
      break;
    }
    default:
      break;
  }
}

void DesktopWindowTreeHostOzone::DispatchEvent(ui::Event* event) {
  switch (event->type()) {
    case ui::ET_MOUSEWHEEL:
    case ui::ET_MOUSE_RELEASED:
    case ui::ET_MOUSE_MOVED:
    case ui::ET_MOUSE_DRAGGED:
    case ui::ET_MOUSE_PRESSED:
    case ui::ET_MOUSE_ENTERED:
    case ui::ET_MOUSE_EXITED: {
      // In Windows, the native events sent to chrome are separated into client
      // and non-client versions of events, which we record on our LocatedEvent
      // structures. On Desktop Ozone, we emulate the concept of non-client.
      // Before we pass this event to the cross platform event handling
      // framework, we need to make sure it is appropriately marked as
      // non-client if it's in the non client area, or otherwise, we can get
      // into a state where the a window is set as the |mouse_pressed_handler_|
      // in window_event_dispatcher.cc despite the mouse button being released.
      //
      // We can't do this later in the dispatch process because we share that
      // with ash, and ash gets confused about event IS_NON_CLIENT-ness on
      // events, since ash doesn't expect this bit to be set, because it's never
      // been set before. (This works on ash on Windows because none of the
      // mouse events on the ash desktop are clicking in what Windows considers
      // to be a non client area.) Likewise, we won't want to do the following
      // in any WindowTreeHost that hosts ash.
      ui::MouseEvent* mouseev = static_cast<ui::MouseEvent*>(event);
      if (content_window_ && content_window_->delegate()) {
        int flags = mouseev->flags();
        int hit_test_code =
            content_window_->delegate()->GetNonClientComponent(
                mouseev->location());
        if (hit_test_code != HTCLIENT && hit_test_code != HTNOWHERE)
          flags |= ui::EF_IS_NON_CLIENT;
        mouseev->set_flags(flags);
      }
      break;
    }
    case ui::ET_KEY_PRESSED:
    case ui::ET_KEY_RELEASED: {
      GetInputMethod()->DispatchKeyEvent(static_cast<ui::KeyEvent*>(event));
      break;
    }

    default:
      break;
  }

  SendEventToProcessor(event);
}

////////////////////////////////////////////////////////////////////////////////
// DesktopWindowTreeHostOzone, private:

void DesktopWindowTreeHostOzone::InitOzoneWindow(
    const Widget::InitParams& params) {
  const gfx::Rect& bounds_in_pixels = ToPixelRect(params.bounds);
  const gfx::Rect& bounds = gfx::Rect(bounds_in_pixels.origin(),
                                      AdjustSize(bounds_in_pixels.size()));
  platform_window_ =
      ui::OzonePlatform::GetInstance()->CreatePlatformWindow(this, bounds);
  DCHECK(window_);
  // Maintain parent child relation as done in X11 version.
  // If we have a parent, record the parent/child relationship. We use this
  // data during destruction to make sure that when we try to close a parent
  // window, we also destroy all child windows.
  gfx::AcceleratedWidget parent_window = 0;
  if (params.parent && params.parent->GetHost()) {
    parent_window = params.parent->GetHost()->GetAcceleratedWidget();
    window_parent_ = GetHostForAcceleratedWidget(parent_window);
    DCHECK(window_parent_);
    window_parent_->window_children_.insert(this);
  }

  ui::PlatformWindow::PlatformWindowType type =
      ui::PlatformWindow::PLATFORM_WINDOW_UNKNOWN;
  switch (params.type) {
    case Widget::InitParams::TYPE_TOOLTIP: {
      type = ui::PlatformWindow::PLATFORM_WINDOW_TYPE_TOOLTIP;
      break;
    }
    case Widget::InitParams::TYPE_POPUP: {
      type = ui::PlatformWindow::PLATFORM_WINDOW_TYPE_POPUP;
      break;
    }
    case Widget::InitParams::TYPE_MENU: {
      type = ui::PlatformWindow::PLATFORM_WINDOW_TYPE_MENU;
      break;
    }
    case Widget::InitParams::TYPE_BUBBLE: {
      type = ui::PlatformWindow::PLATFORM_WINDOW_TYPE_BUBBLE;
      break;
    }
    case Widget::InitParams::TYPE_WINDOW: {
      type = ui::PlatformWindow::PLATFORM_WINDOW_TYPE_WINDOW;
      break;
    }
    case Widget::InitParams::TYPE_WINDOW_FRAMELESS: {
      type = ui::PlatformWindow::PLATFORM_WINDOW_TYPE_WINDOW_FRAMELESS;
      break;
    }
    default:
      break;
  }

  platform_window_->InitPlatformWindow(type, parent_window);
  // If we have a delegate which is providing a default window icon, use that
  // icon.
  gfx::ImageSkia* window_icon =
      ViewsDelegate::GetInstance()
          ? ViewsDelegate::GetInstance()->GetDefaultWindowIcon()
          : NULL;
  if (window_icon) {
    SetWindowIcons(gfx::ImageSkia(), *window_icon);
  }

  if (params.keep_on_top)
    always_on_top_ = true;
}

void DesktopWindowTreeHostOzone::Relayout() {
  Widget* widget = native_widget_delegate_->AsWidget();
  NonClientView* non_client_view = widget->non_client_view();
  // non_client_view may be NULL, especially during creation.
  if (non_client_view) {
    non_client_view->client_view()->InvalidateLayout();
    non_client_view->InvalidateLayout();
  }
  widget->GetRootView()->Layout();
  ResetWindowRegion();
}

std::list<gfx::AcceleratedWidget>&
DesktopWindowTreeHostOzone::open_windows() {
  if (!open_windows_)
    open_windows_ = new std::list<gfx::AcceleratedWidget>();

  return *open_windows_;
}

gfx::Size DesktopWindowTreeHostOzone::AdjustSize(
    const gfx::Size& requested_size_in_pixels) {
  std::vector<gfx::Display> displays =
      gfx::Screen::GetScreenByType(gfx::SCREEN_TYPE_NATIVE)->GetAllDisplays();
  // Compare against all monitor sizes. The window manager can move the window
  // to whichever monitor it wants.
  for (size_t i = 0; i < displays.size(); ++i) {
    if (requested_size_in_pixels == displays[i].GetSizeInPixel()) {
      return gfx::Size(requested_size_in_pixels.width() - 1,
                       requested_size_in_pixels.height() - 1);
    }
  }

  // Do not request a 0x0 window size.
  gfx::Size size_in_pixels = requested_size_in_pixels;
  size_in_pixels.SetToMax(gfx::Size(1, 1));
  return size_in_pixels;
}

gfx::Rect DesktopWindowTreeHostOzone::ToDIPRect(
    const gfx::Rect& rect_in_pixels) const {
  gfx::RectF rect_in_dip = gfx::RectF(rect_in_pixels);
  GetRootTransform().TransformRectReverse(&rect_in_dip);
  return gfx::ToEnclosingRect(rect_in_dip);
}

gfx::Rect DesktopWindowTreeHostOzone::ToPixelRect(
    const gfx::Rect& rect_in_dip) const {
  gfx::RectF rect_in_pixels = gfx::RectF(rect_in_dip);
  GetRootTransform().TransformRect(&rect_in_pixels);
  return gfx::ToEnclosingRect(rect_in_pixels);
}

void DesktopWindowTreeHostOzone::ResetWindowRegion() {
  if (custom_window_shape_)
    return;

  gfx::Path window_mask;
  const gfx::Rect& bounds_in_pixels = platform_window_->GetBounds();
  if (!IsMaximized() && !IsFullscreen()) {
    views::Widget* widget = native_widget_delegate_->AsWidget();
    if (widget->non_client_view()) {
      // Some frame views define a custom (non-rectangular) window mask. If
      // so, use it to define the window shape. If not, fall through.
      widget->non_client_view()->GetWindowMask(bounds_in_pixels.size(),
                                               &window_mask);
    }
  }

  if (!window_mask.countPoints()) {
    // TODO(kalyan): handle the case where window has system borders..
    SkRect rect = { 0,
                    0,
                    SkIntToScalar(bounds_in_pixels.width()),
                    SkIntToScalar(bounds_in_pixels.height()) };
    window_mask.addRect(rect);
  }

  platform_window_->SetWindowShape(window_mask);
}

// static
VIEWS_EXPORT ui::NativeTheme*
DesktopWindowTreeHost::GetNativeTheme(aura::Window* window) {
  const views::LinuxUI* linux_ui = views::LinuxUI::instance();
  if (linux_ui) {
    ui::NativeTheme* native_theme = linux_ui->GetNativeTheme(window);
    if (native_theme)
      return native_theme;
  }

  return ui::NativeTheme::GetInstanceForWeb();
}

}  // namespace views
