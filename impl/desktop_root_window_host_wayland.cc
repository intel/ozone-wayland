// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/desktop_root_window_host_wayland.h"

#include "ozone/impl/ozone_display.h"

#include "base/bind.h"
#include "ui/aura/client/cursor_client.h"
#include "ui/aura/client/focus_client.h"
#include "ui/aura/root_window.h"
#include "ui/aura/window_property.h"
#include "ui/base/dragdrop/os_exchange_data_provider_aura.h"
#include "ui/events/event_utils.h"
#include "ui/gfx/ozone/surface_factory_ozone.h"
#include "ui/gfx/insets.h"
#include "ui/native_theme/native_theme.h"
#include "ui/views/corewm/corewm_switches.h"
#include "ui/views/corewm/tooltip_aura.h"
#include "ui/views/linux_ui/linux_ui.h"
#include "ui/views/widget/desktop_aura/desktop_dispatcher_client.h"
#include "ui/views/widget/desktop_aura/desktop_native_cursor_manager.h"
#include "ui/views/widget/desktop_aura/desktop_native_widget_aura.h"
#include "ui/views/widget/desktop_aura/desktop_screen_position_client.h"
#include "ozone/impl/desktop_drag_drop_client_wayland.h"

namespace views {
// static
ui::NativeTheme* DesktopRootWindowHost::GetNativeTheme(aura::Window* window) {
  const views::LinuxUI* linux_ui = views::LinuxUI::instance();
  if (linux_ui) {
    ui::NativeTheme* native_theme = linux_ui->GetNativeTheme();
    if (native_theme)
      return native_theme;
  }

  return ui::NativeTheme::instance();
}

}

namespace ozonewayland {

using namespace views;

DesktopRootWindowHostWayland* DesktopRootWindowHostWayland::g_current_capture =
    NULL;

DEFINE_WINDOW_PROPERTY_KEY(
    aura::Window*, kViewsWindowForRootWindow, NULL);

DEFINE_WINDOW_PROPERTY_KEY(
    DesktopRootWindowHostWayland*, kHostForRootWindow, NULL);

DesktopRootWindowHostWayland::DesktopRootWindowHostWayland(
    internal::NativeWidgetDelegate* native_widget_delegate,
    views::DesktopNativeWidgetAura* desktop_native_widget_aura)
    : close_widget_factory_(this),
      root_window_(NULL),
      drag_drop_client_(NULL),
      native_widget_delegate_(native_widget_delegate),
      desktop_native_widget_aura_(desktop_native_widget_aura),
      state_(Uninitialized) {
}

DesktopRootWindowHostWayland::~DesktopRootWindowHostWayland() {
  root_window_->ClearProperty(kHostForRootWindow);
  desktop_native_widget_aura_->OnDesktopRootWindowHostDestroyed(root_window_);
}

////////////////////////////////////////////////////////////////////////////////
// DesktopRootWindowHostWayland, private:

void DesktopRootWindowHostWayland::InitWaylandWindow(
    const Widget::InitParams& params) {

  gfx::SurfaceFactoryOzone* surface_factory =
          gfx::SurfaceFactoryOzone::GetInstance();
  window_ = surface_factory->GetAcceleratedWidget();
  surface_factory->AttemptToResizeAcceleratedWidget(window_, params.bounds);
}

void DesktopRootWindowHostWayland::HandleNativeWidgetActivationChanged(
    bool active) {
  native_widget_delegate_->OnNativeWidgetActivationChanged(active);
  native_widget_delegate_->AsWidget()->GetRootView()->SchedulePaint();
}

bool DesktopRootWindowHostWayland::IsWindowManagerPresent() {
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// DesktopRootWindowHostWayland, DesktopRootWindowHost implementation:

void DesktopRootWindowHostWayland::Init(
    aura::Window* content_window,
    const Widget::InitParams& params,
    aura::RootWindow::CreateParams* rw_create_params) {
  content_window_ = content_window;

  // TODO(erg): Check whether we *should* be building a RootWindowHost here, or
  // whether we should be proxying requests to another DRWHL.

  // In some situations, views tries to make a zero sized window, and that
  // makes us crash. Make sure we have valid sizes.
  Widget::InitParams sanitized_params = params;
  if (sanitized_params.bounds.width() == 0)
    sanitized_params.bounds.set_width(100);
  if (sanitized_params.bounds.height() == 0)
    sanitized_params.bounds.set_height(100);

  InitWaylandWindow(sanitized_params);

  rw_create_params->initial_bounds = bounds_;
  rw_create_params->host = this;
}

void DesktopRootWindowHostWayland::OnRootWindowCreated(
    aura::RootWindow* root,
    const Widget::InitParams& params) {
  root_window_ = root;

  root_window_->SetProperty(kViewsWindowForRootWindow, content_window_);
  root_window_->SetProperty(kHostForRootWindow, this);
  root_window_host_delegate_ = root_window_;

  // If we're given a parent, we need to mark ourselves as transient to another
  // window. Otherwise activation gets screwy.
  gfx::NativeView parent = params.parent;
  if (!params.child && params.parent)
    parent->AddTransientChild(content_window_);

  // Add DesktopRootWindowHostWayland as dispatcher.
  base::MessagePumpOzone::Current()->AddDispatcherForRootWindow(this);
}

scoped_ptr<views::corewm::Tooltip> DesktopRootWindowHostWayland::CreateTooltip() {
  return scoped_ptr<corewm::Tooltip>(
             new corewm::TooltipAura(gfx::SCREEN_TYPE_NATIVE));
}

scoped_ptr<aura::client::DragDropClient>
DesktopRootWindowHostWayland::CreateDragDropClient(
    views::DesktopNativeCursorManager* cursor_manager) {
  drag_drop_client_ = new DesktopDragDropClientWayland(root_window_);
  return scoped_ptr<aura::client::DragDropClient>(drag_drop_client_).Pass();
}

void DesktopRootWindowHostWayland::Close() {
  // TODO(erg): Might need to do additional hiding tasks here.

  if (!close_widget_factory_.HasWeakPtrs()) {
    // And we delay the close so that if we are called from an ATL callback,
    // we don't destroy the window before the callback returned (as the caller
    // may delete ourselves on destroy and the ATL callback would still
    // dereference us when the callback returns).
    base::MessageLoop::current()->PostTask(
        FROM_HERE,
        base::Bind(&DesktopRootWindowHostWayland::CloseNow,
                   close_widget_factory_.GetWeakPtr()));
  }
}

void DesktopRootWindowHostWayland::CloseNow() {
  native_widget_delegate_->OnNativeWidgetDestroying();

  // TODO: Actually free our native resources.

  desktop_native_widget_aura_->OnHostClosed();
}

aura::RootWindowHost* DesktopRootWindowHostWayland::AsRootWindowHost() {
  return this;
}

void DesktopRootWindowHostWayland::ShowWindowWithState(
    ui::WindowShowState show_state) {
  if (show_state != ui::SHOW_STATE_DEFAULT &&
      show_state != ui::SHOW_STATE_NORMAL) {
    // Only forwarding to Show().
    NOTIMPLEMENTED();
  }

  Show();
}

void DesktopRootWindowHostWayland::ShowMaximizedWithBounds(
    const gfx::Rect& restored_bounds) {
  // TODO(erg):
  NOTIMPLEMENTED();

  // TODO(erg): We shouldn't completely fall down here.
  Show();
}

bool DesktopRootWindowHostWayland::IsVisible() const {
  return state_ & Visible;
}

void DesktopRootWindowHostWayland::SetSize(const gfx::Size& size) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostWayland::CenterWindow(const gfx::Size& size) {
  gfx::Rect parent_bounds = GetWorkAreaBoundsInScreen();

  // If |window_|'s transient parent bounds are big enough to contain |size|,
  // use them instead.
  if (content_window_->transient_parent()) {
    gfx::Rect transient_parent_rect =
        content_window_->transient_parent()->GetBoundsInScreen();
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

void DesktopRootWindowHostWayland::GetWindowPlacement(
    gfx::Rect* bounds,
    ui::WindowShowState* show_state) const {
  *bounds = bounds_;

  // TODO(erg): This needs a better implementation. For now, we're just pass
  // back the normal state until we keep track of this.
  *show_state = ui::SHOW_STATE_NORMAL;
}

gfx::Rect DesktopRootWindowHostWayland::GetWindowBoundsInScreen() const {
  return bounds_;
}

gfx::Rect DesktopRootWindowHostWayland::GetClientAreaBoundsInScreen() const {
  // TODO(erg): The NativeWidgetAura version returns |bounds_|, claiming its
  // needed for View::ConvertPointToScreen() to work
  // correctly. DesktopRootWindowHostWin::GetClientAreaBoundsInScreen() just
  // asks windows what it thinks the client rect is.
  //
  // Attempts to calculate the rect by asking the NonClientFrameView what it
  // thought its GetBoundsForClientView() were broke combobox drop down
  // placement.
  return bounds_;
}

gfx::Rect DesktopRootWindowHostWayland::GetRestoredBounds() const {
  // TODO(erg):
  NOTIMPLEMENTED();
  return gfx::Rect();
}

gfx::Rect DesktopRootWindowHostWayland::GetWorkAreaBoundsInScreen() const {
  NOTIMPLEMENTED();
  return gfx::Rect(0, 0, 10, 10);
}

void DesktopRootWindowHostWayland::SetShape(gfx::NativeRegion native_region) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostWayland::Activate() {
  if (state_ & Active)
    return;

  state_ |= Active;
  OzoneDisplay::GetInstance()->SetWidgetState(window_, OzoneDisplay::Active);
}

void DesktopRootWindowHostWayland::Deactivate() {
  if (!(state_ & Active))
    return;

  state_ &= ~Active;
  OzoneDisplay::GetInstance()->SetWidgetState(window_, OzoneDisplay::InActive);
}

bool DesktopRootWindowHostWayland::IsActive() const {
  return state_ & Active;
}

void DesktopRootWindowHostWayland::Maximize() {
  if (state_ & Maximized)
    return;

  state_ |= Maximized;
  state_ &= ~Minimized;
  state_ &= ~Normal;
  OzoneDisplay::GetInstance()->SetWidgetState(window_, OzoneDisplay::Maximized);
}

void DesktopRootWindowHostWayland::Minimize() {
  if (state_ & Minimized)
    return;

  state_ &= ~Maximized;
  state_ |= Minimized;
  state_ &= ~Normal;
  OzoneDisplay::GetInstance()->SetWidgetState(window_, OzoneDisplay::Minimized);
}

void DesktopRootWindowHostWayland::Restore() {
  if (state_ & Normal)
    return;

  state_ &= ~Maximized;
  state_ &= ~Minimized;
  state_ |= Normal;
  OzoneDisplay::GetInstance()->SetWidgetState(window_, OzoneDisplay::Restore);
}

bool DesktopRootWindowHostWayland::IsMaximized() const {
  return state_ & Maximized;
}

bool DesktopRootWindowHostWayland::IsMinimized() const {
  return state_ & Minimized;
}

void DesktopRootWindowHostWayland::OnCaptureReleased() {
  native_widget_delegate_->OnMouseCaptureLost();
  g_current_capture = NULL;
}

void DesktopRootWindowHostWayland::DispatchMouseEvent(ui::MouseEvent* event) {
  if (!g_current_capture || g_current_capture == this) {
    root_window_host_delegate_->OnHostMouseEvent(event);
  } else {
    // Another DesktopRootWindowHostX11 has installed itself as
    // capture. Translate the event's location and dispatch to the other.
    event->ConvertLocationToTarget(root_window_,
                                   g_current_capture->root_window_);
    g_current_capture->root_window_host_delegate_->OnHostMouseEvent(event);
  }
}

bool DesktopRootWindowHostWayland::HasCapture() const {
  return g_current_capture == this;
}

bool DesktopRootWindowHostWayland::IsAlwaysOnTop() const {
  NOTIMPLEMENTED();
  return false;
}

void DesktopRootWindowHostWayland::SetAlwaysOnTop(bool always_on_top) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostWayland::SetWindowTitle(const string16& title) {
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostWayland::ClearNativeFocus() {
  // This method is weird and misnamed. Instead of clearing the native focus,
  // it sets the focus to our |content_window_|, which will trigger a cascade
  // of focus changes into views.
  if (content_window_ && aura::client::GetFocusClient(content_window_) &&
      content_window_->Contains(
          aura::client::GetFocusClient(content_window_)->GetFocusedWindow())) {
    aura::client::GetFocusClient(content_window_)->FocusWindow(content_window_);
  }
}

Widget::MoveLoopResult DesktopRootWindowHostWayland::RunMoveLoop(
    const gfx::Vector2d& drag_offset,
    Widget::MoveLoopSource source,
    Widget::MoveLoopEscapeBehavior escape_behavior) {
  NOTIMPLEMENTED();
  return Widget::MOVE_LOOP_SUCCESSFUL;
}

void DesktopRootWindowHostWayland::EndMoveLoop() {
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostWayland::SetVisibilityChangedAnimationsEnabled(
    bool value) {
  // Much like the previous NativeWidgetGtk, we don't have anything to do here.
}

bool DesktopRootWindowHostWayland::ShouldUseNativeFrame() {
  return false;
}

void DesktopRootWindowHostWayland::FrameTypeChanged() {
}

NonClientFrameView* DesktopRootWindowHostWayland::CreateNonClientFrameView() {
  return NULL;
}

void DesktopRootWindowHostWayland::SetFullscreen(bool fullscreen) {
  if ((state_ & FullScreen) == fullscreen)
    return;

  if (fullscreen)
    state_ |= FullScreen;
  else
    state_ &= ~FullScreen;

  OzoneDisplay::GetInstance()->SetWidgetState(window_, OzoneDisplay::FullScreen);
}

bool DesktopRootWindowHostWayland::IsFullscreen() const {
  return state_ & FullScreen;
}

void DesktopRootWindowHostWayland::SetOpacity(unsigned char opacity) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostWayland::SetWindowIcons(
    const gfx::ImageSkia& window_icon, const gfx::ImageSkia& app_icon) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostWayland::InitModalType(ui::ModalType modal_type) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostWayland::FlashFrame(bool flash_frame) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostWayland::SetInactiveRenderingDisabled(
    bool disable_inactive) {
}

void DesktopRootWindowHostWayland::OnRootViewLayout() const {
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostWayland::OnNativeWidgetFocus() {
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostWayland::OnNativeWidgetBlur() {
  NOTIMPLEMENTED();
}

bool DesktopRootWindowHostWayland::IsAnimatingClosed() const {
  NOTIMPLEMENTED();
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// DesktopRootWindowHostWayland, aura::RootWindowHost implementation:

void DesktopRootWindowHostWayland::SetDelegate(
    aura::RootWindowHostDelegate* delegate) {
  root_window_host_delegate_ = delegate;
}

aura::RootWindow* DesktopRootWindowHostWayland::GetRootWindow() {
  return root_window_;
}

gfx::AcceleratedWidget DesktopRootWindowHostWayland::GetAcceleratedWidget() {
  return window_;
}

void DesktopRootWindowHostWayland::Show() {
  if (state_ & Visible)
    return;

  state_ |= Visible;
  OzoneDisplay::GetInstance()->SetWidgetState(window_, OzoneDisplay::Show);
}

void DesktopRootWindowHostWayland::Hide() {
  if (!(state_ & Visible))
    return;

  state_ &= ~Visible;
  OzoneDisplay::GetInstance()->SetWidgetState(window_, OzoneDisplay::Hide);
}

void DesktopRootWindowHostWayland::ToggleFullScreen() {
  SetFullscreen(!(state_ & FullScreen));
}

gfx::Rect DesktopRootWindowHostWayland::GetBounds() const {
  return bounds_;
}

void DesktopRootWindowHostWayland::SetBounds(const gfx::Rect& bounds) {
  bool origin_changed = bounds_.origin() != bounds.origin();
  bool size_changed = bounds_.size() != bounds.size();

  bounds_ = bounds;

  if (origin_changed)
    native_widget_delegate_->AsWidget()->OnNativeWidgetMove();
  if (size_changed) {
    root_window_host_delegate_->OnHostResized(bounds.size());
    OzoneDisplay::GetInstance()->SetWidgetState(window_,
                                                OzoneDisplay::Resize,
                                                bounds.width(),
                                                bounds.height());
  } else
    root_window_host_delegate_->OnHostPaint(gfx::Rect(bounds.size()));
}

gfx::Insets DesktopRootWindowHostWayland::GetInsets() const {
  return gfx::Insets();
}

void DesktopRootWindowHostWayland::SetInsets(const gfx::Insets& insets) {
}

gfx::Point DesktopRootWindowHostWayland::GetLocationOnNativeScreen() const {
  return bounds_.origin();
}

void DesktopRootWindowHostWayland::SetCapture() {
  // This is vaguely based on the old NativeWidgetGtk implementation.
  //
  // X11's XPointerGrab() shouldn't be used for everything; it doesn't map
  // cleanly to Windows' SetCapture(). GTK only provides a separate concept of
  // a grab that wasn't the X11 pointer grab, but was instead a manual
  // redirection of the event. (You need to drop into GDK if you want to
  // perform a raw X11 grab).

  if (g_current_capture)
    g_current_capture->OnCaptureReleased();

  g_current_capture = this;

  // TODO(erg): In addition to the above, NativeWidgetGtk performs a full X
  // pointer grab when our NativeWidget is of type Menu. However, things work
  // without it. Clicking inside a chrome window causes a release capture, and
  // clicking outside causes an activation change. Since previous attempts at
  // using XPointerGrab() to implement this have locked my X server, I'm going
  // to skip this for now.
}

void DesktopRootWindowHostWayland::ReleaseCapture() {
  if (g_current_capture)
    g_current_capture->OnCaptureReleased();
}

void DesktopRootWindowHostWayland::SetCursor(gfx::NativeCursor cursor) {
  NOTIMPLEMENTED();
}

bool DesktopRootWindowHostWayland::QueryMouseLocation(
    gfx::Point* location_return) {
  NOTIMPLEMENTED();
  return false;
}

bool DesktopRootWindowHostWayland::ConfineCursorToRootWindow() {
  NOTIMPLEMENTED();
  return false;
}

void DesktopRootWindowHostWayland::UnConfineCursor() {
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostWayland::OnCursorVisibilityChanged(bool show) {
  // TODO(erg): Conditional on us enabling touch on desktop linux builds, do
  // the same tap-to-click disabling here that chromeos does.
}

void DesktopRootWindowHostWayland::MoveCursorTo(const gfx::Point& location) {
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostWayland::SetFocusWhenShown(bool focus_when_shown) {
  NOTIMPLEMENTED();
}

bool DesktopRootWindowHostWayland::GrabSnapshot(
      const gfx::Rect& snapshot_bounds,
      std::vector<unsigned char>* png_representation) {
  NOTIMPLEMENTED();
  return false;
}

void DesktopRootWindowHostWayland::PostNativeEvent(
    const base::NativeEvent& native_event) {
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostWayland::OnDeviceScaleFactorChanged(
    float device_scale_factor) {
}

void DesktopRootWindowHostWayland::PrepareForShutdown() {
}

////////////////////////////////////////////////////////////////////////////////
// DesktopRootWindowHostWayland, MessageLoop::Dispatcher implementation:

bool DesktopRootWindowHostWayland::Dispatch(const base::NativeEvent& ne) {
  ui::Event* event = static_cast<ui::Event*>(ne);
  ui::EventType type = ui::EventTypeFromNative(ne);

  switch (type) {
    case ui::ET_TOUCH_MOVED:
    case ui::ET_TOUCH_PRESSED:
    case ui::ET_TOUCH_CANCELLED:
    case ui::ET_TOUCH_RELEASED: {
      ui::TouchEvent touchev(event);
      root_window_host_delegate_->OnHostTouchEvent(&touchev);
      break;
    }
    case ui::ET_KEY_PRESSED: {
      ui::KeyEvent keydown_event(event, false);
      root_window_host_delegate_->OnHostKeyEvent(&keydown_event);
      break;
    }
    case ui::ET_KEY_RELEASED: {
      ui::KeyEvent keyup_event(event, false);
      root_window_host_delegate_->OnHostKeyEvent(&keyup_event);
      break;
    }
    case ui::ET_MOUSEWHEEL: {
      ui::MouseWheelEvent wheelev(event);
      DispatchMouseEvent(&wheelev);
      break;
    }
    case ui::ET_MOUSE_MOVED:
    case ui::ET_MOUSE_DRAGGED:
    case ui::ET_MOUSE_PRESSED:
    case ui::ET_MOUSE_RELEASED:
    case ui::ET_MOUSE_ENTERED:
    case ui::ET_MOUSE_EXITED: {
      ui::MouseEvent mouseev(event);
      DispatchMouseEvent(&mouseev);
      break;
    }
    case ui::ET_SCROLL_FLING_START:
    case ui::ET_SCROLL_FLING_CANCEL:
    case ui::ET_SCROLL: {
      ui::ScrollEvent scrollev(event);
      root_window_host_delegate_->OnHostScrollEvent(&scrollev);
      break;
    }
    case ui::ET_UMA_DATA:
      break;
    case ui::ET_UNKNOWN:
      break;
    default:
      NOTIMPLEMENTED() << "DesktopRootWindowHostWayland: unknown event type.";
  }
  return true;
}

}  // namespace ozonewayland
