// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/desktop_aura/desktop_window_tree_host_wayland.h"

#include <string>

#include "base/bind.h"
#include "base/strings/utf_string_conversions.h"
#include "ozone/ui/desktop_aura/desktop_drag_drop_client_wayland.h"
#include "ozone/ui/desktop_aura/desktop_screen_wayland.h"
#include "ozone/ui/desktop_aura/window_tree_host_delegate_wayland.h"
#include "ozone/ui/events/window_state_change_handler.h"
#include "ui/aura/client/cursor_client.h"
#include "ui/aura/client/focus_client.h"
#include "ui/aura/window_property.h"
#include "ui/base/dragdrop/os_exchange_data_provider_aura.h"
#include "ui/base/ime/composition_text.h"
#include "ui/base/ime/input_method.h"
#include "ui/base/ime/input_method_auralinux.h"
#include "ui/events/event_utils.h"
#include "ui/gfx/insets.h"
#include "ui/native_theme/native_theme.h"
#include "ui/ozone/public/surface_factory_ozone.h"
#include "ui/views/corewm/tooltip_aura.h"
#include "ui/views/ime/input_method.h"
#include "ui/views/linux_ui/linux_ui.h"
#include "ui/views/views_export.h"
#include "ui/views/widget/desktop_aura/desktop_dispatcher_client.h"
#include "ui/views/widget/desktop_aura/desktop_native_cursor_manager.h"
#include "ui/views/widget/desktop_aura/desktop_native_widget_aura.h"
#include "ui/views/widget/desktop_aura/desktop_screen_position_client.h"
#include "ui/wm/core/input_method_event_filter.h"
#include "ui/wm/core/window_util.h"
#include "ui/wm/public/window_move_client.h"

namespace views {

WindowTreeHostDelegateWayland*
    DesktopWindowTreeHostWayland::g_delegate_ozone_wayland_ = NULL;

DEFINE_WINDOW_PROPERTY_KEY(
    aura::Window*, kViewsWindowForRootWindow, NULL);

DEFINE_WINDOW_PROPERTY_KEY(
    DesktopWindowTreeHostWayland*, kHostForRootWindow, NULL);

DesktopWindowTreeHostWayland::DesktopWindowTreeHostWayland(
    internal::NativeWidgetDelegate* native_widget_delegate,
    DesktopNativeWidgetAura* desktop_native_widget_aura)
    : aura::WindowTreeHost(),
      state_(Uninitialized),
      bounds_(0, 0, 0, 0),
      previous_bounds_(0, 0, 0, 0),
      previous_maximize_bounds_(0, 0, 0, 0),
      window_(0),
      title_(base::string16()),
      close_widget_factory_(this),
      drag_drop_client_(NULL),
      native_widget_delegate_(native_widget_delegate),
      content_window_(NULL),
      desktop_native_widget_aura_(desktop_native_widget_aura),
      window_parent_(NULL),
      window_children_() {
}

DesktopWindowTreeHostWayland::~DesktopWindowTreeHostWayland() {
  window()->ClearProperty(kHostForRootWindow);
  aura::client::SetWindowMoveClient(window(), NULL);
  desktop_native_widget_aura_->OnDesktopWindowTreeHostDestroyed(this);
  DestroyDispatcher();
}

// static
DesktopWindowTreeHostWayland*
DesktopWindowTreeHostWayland::GetHostForAcceleratedWidget(
    gfx::AcceleratedWidget widget) {
  aura::WindowTreeHost* host =
      aura::WindowTreeHost::GetForAcceleratedWidget(widget);

  return host ? host->window()->GetProperty(kHostForRootWindow) : NULL;
}

// static
const std::vector<aura::Window*>&
DesktopWindowTreeHostWayland::GetAllOpenWindows() {
  DCHECK(g_delegate_ozone_wayland_);
  return g_delegate_ozone_wayland_->GetAllOpenWindows();
}

// static
aura::Window*
DesktopWindowTreeHostWayland::GetContentWindowForAcceleratedWidget(
    gfx::AcceleratedWidget widget) {
  aura::WindowTreeHost* host =
      aura::WindowTreeHost::GetForAcceleratedWidget(widget);

  return host ?
      host->window()->GetProperty(kViewsWindowForRootWindow) : NULL;
}

gfx::Rect DesktopWindowTreeHostWayland::GetBoundsInScreen() const {
  return bounds_;
}

////////////////////////////////////////////////////////////////////////////////
// DesktopWindowTreeHostWayland, private:

void DesktopWindowTreeHostWayland::InitWaylandWindow(
    const Widget::InitParams& params) {
  ui::SurfaceFactoryOzone* surface_factory =
          ui::SurfaceFactoryOzone::GetInstance();
  window_ = surface_factory->GetAcceleratedWidget();
  // Maintain parent child relation as done in X11 version.
  // If we have a parent, record the parent/child relationship. We use this
  // data during destruction to make sure that when we try to close a parent
  // window, we also destroy all child windows.
  if (params.parent && params.parent->GetHost()) {
    gfx::AcceleratedWidget windowId = params.parent->GetHost()->
        GetAcceleratedWidget();
    window_parent_ = GetHostForAcceleratedWidget(windowId);
    DCHECK(window_parent_);
    window_parent_->window_children_.insert(this);
  }

  bounds_ = params.bounds;
  ui::WindowStateChangeHandler* state_handler =
      ui::WindowStateChangeHandler::GetInstance();
  switch (params.type) {
    case Widget::InitParams::TYPE_TOOLTIP:
    case Widget::InitParams::TYPE_POPUP:
    case Widget::InitParams::TYPE_MENU: {
      // Wayland surfaces don't know their position on the screen and transient
      // surfaces always require a parent surface for relative placement. Here
      // there's a catch because content_shell menus don't have parent and
      // therefore we use root window to calculate their position.
      DesktopWindowTreeHostWayland* parent = window_parent_;
      if (!parent)
        parent = GetHostForAcceleratedWidget(
                     g_delegate_ozone_wayland_->GetActiveWindow()->window_);

      // Transient type expects a position relative to the parent
      gfx::Point transientPos(bounds_.x() - parent->bounds_.x(),
                              bounds_.y() - parent->bounds_.y());

      // Different platforms implement different input grab pointer behaviors
      // on Chromium. While the Linux GTK+ grab button clicks but not the
      // pointer movement, the MS Windows implementation don't implement any
      // pointer grab. In here we're using another different behavior for
      // Chromium, but which is the common sense on most Wayland UI
      // environments, where the input pointer is grabbed as a whole when a
      // menu type of window is opened. I.e. both pointer clicks and movements
      // will be routed only to the newly created window (grab installed). For
      // more information please refer to the Wayland protocol.
      state_handler->SetWidgetAttributes(window_,
                                         parent->window_,
                                         transientPos.x(),
                                         transientPos.y(),
                                         ui::POPUP);
      break;
    }
    case Widget::InitParams::TYPE_BUBBLE:
    case Widget::InitParams::TYPE_WINDOW:
      state_handler->SetWidgetAttributes(window_,
                                         0,
                                         0,
                                         0,
                                         ui::WINDOW);
      break;
    case Widget::InitParams::TYPE_WINDOW_FRAMELESS:
      NOTIMPLEMENTED();
      break;
    default:
      break;
  }

  CreateCompositor(GetAcceleratedWidget());
}

////////////////////////////////////////////////////////////////////////////////
// DesktopWindowTreeHostWayland, DesktopWindowTreeHost implementation:

void DesktopWindowTreeHostWayland::Init(
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

  InitWaylandWindow(sanitized_params);
}

void DesktopWindowTreeHostWayland::OnNativeWidgetCreated(
    const Widget::InitParams& params) {
  window()->SetProperty(kViewsWindowForRootWindow, content_window_);
  window()->SetProperty(kHostForRootWindow, this);

  // If we're given a parent, we need to mark ourselves as transient to another
  // window. Otherwise activation gets screwy.
  gfx::NativeView parent = params.parent;
  if (!params.child && params.parent)
    wm::AddTransientChild(parent, content_window_);

  native_widget_delegate_->OnNativeWidgetCreated(true);

  if (!g_delegate_ozone_wayland_)
    g_delegate_ozone_wayland_ = new WindowTreeHostDelegateWayland();

  g_delegate_ozone_wayland_->OnRootWindowCreated(window_);

  // Add DesktopWindowTreeHostWayland as dispatcher.
  bool root_window = params.type == Widget::InitParams::TYPE_BUBBLE ||
      params.type == Widget::InitParams::TYPE_WINDOW ||
      params.type == Widget::InitParams::TYPE_WINDOW_FRAMELESS;
  if (!window_parent_ && root_window)
    g_delegate_ozone_wayland_->SetActiveWindow(this);
}

scoped_ptr<corewm::Tooltip>
DesktopWindowTreeHostWayland::CreateTooltip() {
  return scoped_ptr<corewm::Tooltip>(
             new corewm::TooltipAura(gfx::SCREEN_TYPE_NATIVE));
}

scoped_ptr<aura::client::DragDropClient>
DesktopWindowTreeHostWayland::CreateDragDropClient(
    DesktopNativeCursorManager* cursor_manager) {
  drag_drop_client_ = new DesktopDragDropClientWayland(window());
  return scoped_ptr<aura::client::DragDropClient>(drag_drop_client_).Pass();
}

void DesktopWindowTreeHostWayland::Close() {
  if (!close_widget_factory_.HasWeakPtrs()) {
    // And we delay the close so that if we are called from an ATL callback,
    // we don't destroy the window before the callback returned (as the caller
    // may delete ourselves on destroy and the ATL callback would still
    // dereference us when the callback returns).
    base::MessageLoop::current()->PostTask(
        FROM_HERE,
        base::Bind(&DesktopWindowTreeHostWayland::CloseNow,
                   close_widget_factory_.GetWeakPtr()));
  }
}

void DesktopWindowTreeHostWayland::CloseNow() {
  DCHECK(g_delegate_ozone_wayland_);
  unsigned widgetId = window_;
  native_widget_delegate_->OnNativeWidgetDestroying();

  // If we have children, close them. Use a copy for iteration because they'll
  // remove themselves.
  std::set<DesktopWindowTreeHostWayland*> window_children_copy =
      window_children_;
  for (std::set<DesktopWindowTreeHostWayland*>::iterator it =
           window_children_copy.begin(); it != window_children_copy.end();
       ++it) {
    (*it)->CloseNow();
  }
  DCHECK(window_children_.empty());

  // If we have a parent, remove ourselves from its children list.
  if (window_parent_)
    window_parent_->window_children_.erase(this);

  g_delegate_ozone_wayland_->OnRootWindowClosed(widgetId);
  // Destroy the compositor before destroying the window since shutdown
  // may try to swap, and the swap without a window causes an error, which
  // causes a crash with in-process renderer.
  DestroyCompositor();
  window_parent_ = NULL;
  if (!g_delegate_ozone_wayland_->GetActiveWindow()) {
    // We have no open windows, free g_delegate_ozone_wayland_.
    delete g_delegate_ozone_wayland_;
    g_delegate_ozone_wayland_ = NULL;
  }

  desktop_native_widget_aura_->OnHostClosed();
}

aura::WindowTreeHost* DesktopWindowTreeHostWayland::AsWindowTreeHost() {
  return this;
}

void DesktopWindowTreeHostWayland::ShowWindowWithState(
    ui::WindowShowState show_state) {
  if (show_state != ui::SHOW_STATE_DEFAULT &&
      show_state != ui::SHOW_STATE_NORMAL) {
    // Only forwarding to Show().
    NOTIMPLEMENTED();
  }

  Show();
  if (show_state == ui::SHOW_STATE_MAXIMIZED)
    Maximize();

  // Set initial focus for root window.
  if (!window_parent_)
    native_widget_delegate_->AsWidget()->SetInitialFocus(show_state);
}

void DesktopWindowTreeHostWayland::ShowMaximizedWithBounds(
    const gfx::Rect& restored_bounds) {
  Maximize();
  previous_bounds_ = restored_bounds;
  Show();
}

bool DesktopWindowTreeHostWayland::IsVisible() const {
  return state_ & Visible;
}

void DesktopWindowTreeHostWayland::SetSize(const gfx::Size& size) {
  gfx::Rect new_bounds = bounds_;
  new_bounds.set_size(size);
  SetBounds(new_bounds);
}

void DesktopWindowTreeHostWayland::StackAtTop() {
}

void DesktopWindowTreeHostWayland::CenterWindow(const gfx::Size& size) {
  gfx::Rect parent_bounds = GetWorkAreaBoundsInScreen();

  // If |window_|'s transient parent bounds are big enough to contain |size|,
  // use them instead.
  if (wm::GetTransientParent(content_window_)) {
    gfx::Rect transient_parent_rect =
        wm::GetTransientParent(content_window_)->GetBoundsInScreen();
    if (transient_parent_rect.height() >= size.height() &&
        transient_parent_rect.width() >= size.width()) {
      parent_bounds = transient_parent_rect;
    }
  }

  gfx::Rect window_bounds(
      parent_bounds.x() + (parent_bounds.width() - size.width()) / 2,
      parent_bounds.y() + (parent_bounds.height() - size.height()) / 2,
      size.width(),
      size.height());
  // Don't size the window bigger than the parent, otherwise the user may not be
  // able to close or move it.
  window_bounds.AdjustToFit(parent_bounds);
  SetBounds(window_bounds);
}

void DesktopWindowTreeHostWayland::GetWindowPlacement(
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

gfx::Rect DesktopWindowTreeHostWayland::GetWindowBoundsInScreen() const {
  return bounds_;
}

gfx::Rect DesktopWindowTreeHostWayland::GetClientAreaBoundsInScreen() const {
  // TODO(erg): The NativeWidgetAura version returns |bounds_|, claiming its
  // needed for View::ConvertPointToScreen() to work
  // correctly. DesktopWindowTreeHostWin::GetClientAreaBoundsInScreen() just
  // asks windows what it thinks the client rect is.
  //
  // Attempts to calculate the rect by asking the NonClientFrameView what it
  // thought its GetBoundsForClientView() were broke combobox drop down
  // placement.
  return bounds_;
}

gfx::Rect DesktopWindowTreeHostWayland::GetRestoredBounds() const {
  if (!previous_bounds_.IsEmpty())
    return previous_bounds_;

  return GetWindowBoundsInScreen();
}

gfx::Rect DesktopWindowTreeHostWayland::GetWorkAreaBoundsInScreen() const {
  // TODO(kalyan): Take into account wm decorations. i.e Dock, panel etc.
  gfx::Screen *screen = gfx::Screen::GetScreenByType(gfx::SCREEN_TYPE_NATIVE);
  if (!screen)
    NOTREACHED() << "Unable to retrieve valid gfx::Screen";

  gfx::Display display = screen->GetPrimaryDisplay();
  return display.bounds();
}

void DesktopWindowTreeHostWayland::SetShape(gfx::NativeRegion native_region) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

void DesktopWindowTreeHostWayland::Activate() {
  if (state_ & Active)
    return;

  state_ |= Active;
  if (state_ & Visible)
    ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(window_,
                                                                ui::ACTIVE);
}

void DesktopWindowTreeHostWayland::Deactivate() {
  if (!(state_ & Active))
    return;

  state_ &= ~Active;
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(window_,
                                                              ui::INACTIVE);
}

bool DesktopWindowTreeHostWayland::IsActive() const {
  return state_ & Active;
}

void DesktopWindowTreeHostWayland::Maximize() {
  if (state_ & Maximized)
    return;
  if (IsMinimized() && !window_parent_)
    native_widget_delegate_->AsWidget()->SetInitialFocus(ui::SHOW_STATE_NORMAL);

  state_ |= Maximized;
  state_ &= ~Minimized;
  state_ &= ~Normal;
  previous_bounds_ = bounds_;
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(window_,
                                                              ui::MAXIMIZED);
}

void DesktopWindowTreeHostWayland::Minimize() {
  if (state_ & Minimized)
    return;

  state_ |= Minimized;
  previous_bounds_ = bounds_;
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(window_,
                                                              ui::MINIMIZED);
}

void DesktopWindowTreeHostWayland::Restore() {
  if (state_ & Normal)
    return;

  if (IsMinimized() && !window_parent_)
    native_widget_delegate_->AsWidget()->SetInitialFocus(ui::SHOW_STATE_NORMAL);
  state_ &= ~Maximized;
  state_ &= ~Minimized;
  state_ |= Normal;
  bounds_ = previous_bounds_;
  previous_bounds_ = gfx::Rect();
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(
      window_, ui::RESTORE, bounds_.width(), bounds_.height());
  native_widget_delegate_->AsWidget()->OnNativeWidgetMove();
  OnHostResized(bounds_.size());
}

bool DesktopWindowTreeHostWayland::IsMaximized() const {
  return state_ & Maximized;
}

bool DesktopWindowTreeHostWayland::IsMinimized() const {
  return state_ & Minimized;
}

void DesktopWindowTreeHostWayland::OnCaptureReleased() {
  OnHostLostWindowCapture();
  native_widget_delegate_->OnMouseCaptureLost();
}

bool DesktopWindowTreeHostWayland::HasCapture() const {
  return g_delegate_ozone_wayland_->GetCurrentCapture() == this;
}

bool DesktopWindowTreeHostWayland::IsAlwaysOnTop() const {
  NOTIMPLEMENTED();
  return false;
}

void DesktopWindowTreeHostWayland::SetVisibleOnAllWorkspaces(
    bool always_visible) {
  NOTIMPLEMENTED();
}

void DesktopWindowTreeHostWayland::SetAlwaysOnTop(bool always_on_top) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

bool DesktopWindowTreeHostWayland::SetWindowTitle(const base::string16& title) {
  if (title.compare(title_)) {
    ui::WindowStateChangeHandler::GetInstance()->SetWidgetTitle(window_, title);
    title_ = title;
    return true;
  }

  return false;
}

void DesktopWindowTreeHostWayland::ClearNativeFocus() {
  // This method is weird and misnamed. Instead of clearing the native focus,
  // it sets the focus to our |content_window_|, which will trigger a cascade
  // of focus changes into views.
  if (content_window_ && aura::client::GetFocusClient(content_window_) &&
      content_window_->Contains(
          aura::client::GetFocusClient(content_window_)->GetFocusedWindow())) {
    aura::client::GetFocusClient(content_window_)->FocusWindow(content_window_);
  }
}

Widget::MoveLoopResult DesktopWindowTreeHostWayland::RunMoveLoop(
    const gfx::Vector2d& drag_offset,
    Widget::MoveLoopSource source,
    Widget::MoveLoopEscapeBehavior escape_behavior) {
  NOTIMPLEMENTED();
  return Widget::MOVE_LOOP_SUCCESSFUL;
}

void DesktopWindowTreeHostWayland::EndMoveLoop() {
  NOTIMPLEMENTED();
}

void DesktopWindowTreeHostWayland::SetVisibilityChangedAnimationsEnabled(
    bool value) {
  // Much like the previous NativeWidgetGtk, we don't have anything to do here.
}

bool DesktopWindowTreeHostWayland::ShouldUseNativeFrame() const {
  return false;
}

bool DesktopWindowTreeHostWayland::ShouldWindowContentsBeTransparent() const {
  return false;
}

void DesktopWindowTreeHostWayland::FrameTypeChanged() {
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

void DesktopWindowTreeHostWayland::SetFullscreen(bool fullscreen) {
  if ((state_ & FullScreen) == fullscreen)
    return;

  if (fullscreen) {
    state_ |= FullScreen;
    state_ &= ~Normal;
  } else {
    state_ &= ~FullScreen;
  }

  if (!(state_ & FullScreen)) {
    if (state_ & Maximized) {
      previous_bounds_ = previous_maximize_bounds_;
      previous_maximize_bounds_ = gfx::Rect();
      ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(
          window_, ui::MAXIMIZED);
    } else {
      Restore();
    }
  } else {
    if (state_ & Maximized)
      previous_maximize_bounds_ = previous_bounds_;

    previous_bounds_ = bounds_;
    gfx::Screen *screen = gfx::Screen::GetScreenByType(gfx::SCREEN_TYPE_NATIVE);
    if (!screen)
      NOTREACHED() << "Unable to retrieve valid gfx::Screen";

    bounds_ = screen->GetPrimaryDisplay().bounds();
    // We could use HandleConfigure in ShellSurface to set the correct bounds of
    // egl window associated with this opaque handle. How ever, this would need
    // to handle race conditions and ensure correct size is set for
    // wl_egl_window_resize before eglsurface is resized. Passing window size
    // attributes already here, ensures that wl_egl_window_resize is resized
    // before eglsurface is resized. This doesn't add any extra overhead as the
    // IPC call needs to be done.
    ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(
        window_, ui::FULLSCREEN, bounds_.width(), bounds_.height());
    OnHostResized(bounds_.size());
  }
}

bool DesktopWindowTreeHostWayland::IsFullscreen() const {
  return state_ & FullScreen;
}

void DesktopWindowTreeHostWayland::SetOpacity(unsigned char opacity) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

void DesktopWindowTreeHostWayland::SetWindowIcons(
    const gfx::ImageSkia& window_icon, const gfx::ImageSkia& app_icon) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

void DesktopWindowTreeHostWayland::InitModalType(ui::ModalType modal_type) {
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

void DesktopWindowTreeHostWayland::FlashFrame(bool flash_frame) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

void DesktopWindowTreeHostWayland::OnRootViewLayout() const {
  NOTIMPLEMENTED();
}

void DesktopWindowTreeHostWayland::OnNativeWidgetFocus() {
  native_widget_delegate_->AsWidget()->GetInputMethod()->OnFocus();
}

void DesktopWindowTreeHostWayland::OnNativeWidgetBlur() {
  if (window_)
    native_widget_delegate_->AsWidget()->GetInputMethod()->OnBlur();
}

bool DesktopWindowTreeHostWayland::IsAnimatingClosed() const {
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// DesktopWindowTreeHostWayland, aura::WindowTreeHost implementation:

ui::EventSource* DesktopWindowTreeHostWayland::GetEventSource() {
  NOTIMPLEMENTED();
  return NULL;
}

gfx::AcceleratedWidget DesktopWindowTreeHostWayland::GetAcceleratedWidget() {
  return window_;
}

void DesktopWindowTreeHostWayland::Show() {
  if (state_ & Visible)
    return;

  // Window is being shown, set the state as active to be able to handle events.
  Activate();

  state_ |= Visible;
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(window_,
                                                              ui::SHOW);
  native_widget_delegate_->OnNativeWidgetVisibilityChanged(true);
}

void DesktopWindowTreeHostWayland::Hide() {
  if (!(state_ & Visible))
    return;

  state_ &= ~Visible;
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetState(window_,
                                                              ui::HIDE);
  native_widget_delegate_->OnNativeWidgetVisibilityChanged(false);
}

gfx::Rect DesktopWindowTreeHostWayland::GetBounds() const {
  return bounds_;
}

void DesktopWindowTreeHostWayland::SetBounds(const gfx::Rect& bounds) {
  bool origin_changed = bounds_.origin() != bounds.origin();
  bool size_changed = bounds_.size() != bounds.size();

  bounds_ = bounds;

  if (origin_changed)
    native_widget_delegate_->AsWidget()->OnNativeWidgetMove();
  if (size_changed)
    OnHostResized(bounds.size());
  else
    compositor()->ScheduleRedrawRect(bounds);
}

gfx::Point DesktopWindowTreeHostWayland::GetLocationOnNativeScreen() const {
  return bounds_.origin();
}

void DesktopWindowTreeHostWayland::SetCapture() {
  g_delegate_ozone_wayland_->SetCapture(this);
}

void DesktopWindowTreeHostWayland::ReleaseCapture() {
  g_delegate_ozone_wayland_->SetCapture(NULL);
}

void DesktopWindowTreeHostWayland::SetCursorNative(gfx::NativeCursor cursor) {
  // TODO(kalyan): Add support for custom cursor.
  ui::WindowStateChangeHandler::GetInstance()->SetWidgetCursor(
      cursor.native_type());
}

void DesktopWindowTreeHostWayland::OnCursorVisibilityChangedNative(bool show) {
  // TODO(erg): Conditional on us enabling touch on desktop linux builds, do
  // the same tap-to-click disabling here that chromeos does.
}

void DesktopWindowTreeHostWayland::MoveCursorToNative(
    const gfx::Point& location) {
  NOTIMPLEMENTED();
}

void DesktopWindowTreeHostWayland::PostNativeEvent(
    const base::NativeEvent& native_event) {
  NOTIMPLEMENTED();
}

void DesktopWindowTreeHostWayland::OnDeviceScaleFactorChanged(
    float device_scale_factor) {
}

////////////////////////////////////////////////////////////////////////////////
// DesktopWindowTreeHostWayland, Private implementation:

void DesktopWindowTreeHostWayland::HandleNativeWidgetActivationChanged(
    bool active) {
  // We can skip during initialization phase.
  if (!state_)
    return;

  if (active)
    OnHostActivated();

  desktop_native_widget_aura_->HandleActivationChanged(active);
  native_widget_delegate_->AsWidget()->GetRootView()->SchedulePaint();
}

void DesktopWindowTreeHostWayland::HandleWindowResize(unsigned width,
                                                      unsigned height) {
  unsigned current_width = bounds_.width();
  unsigned current_height = bounds_.height();
  if ((current_width == width) && (current_height == height)) {
    compositor()->ScheduleRedrawRect(bounds_);
  } else {
    bounds_ = gfx::Rect(bounds_.x(), bounds_.y(), width, height);
    OnHostResized(bounds_.size());
    Widget* widget = native_widget_delegate_->AsWidget();
    NonClientView* non_client_view = widget->non_client_view();
    // non_client_view may be NULL, especially during creation.
    if (non_client_view) {
      non_client_view->client_view()->InvalidateLayout();
      non_client_view->InvalidateLayout();
    }
    widget->GetRootView()->Layout();
  }
}

void DesktopWindowTreeHostWayland::HandleWindowUnminimized() {
  state_ &= ~Minimized;
}

void DesktopWindowTreeHostWayland::HandleCommit(const std::string& text) {
  ui::InputMethodAuraLinux* inputMethod =
      static_cast<ui::InputMethodAuraLinux*>(desktop_native_widget_aura_->
      input_method_event_filter()->input_method());
  inputMethod->OnCommit(base::string16(base::ASCIIToUTF16(text.c_str())));
}

void DesktopWindowTreeHostWayland::HandlePreeditChanged(const std::string& text,
  const std::string& commit) {
  ui::CompositionText composition_text;
  composition_text.text = base::string16(base::ASCIIToUTF16(text.c_str()));
  ui::InputMethodAuraLinux* inputMethod =
      static_cast<ui::InputMethodAuraLinux*>(desktop_native_widget_aura_->
      input_method_event_filter()->input_method());
  inputMethod->OnPreeditChanged(composition_text);
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

  return ui::NativeTheme::instance();
}

}  // namespace views
