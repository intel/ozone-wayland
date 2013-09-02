// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/impl/desktop_root_window_host_wayland.h"

#include "base/bind.h"
#include "base/message_loop/message_pump_ozone.h"
#include "ui/aura/client/screen_position_client.h"
#include "ui/aura/client/user_action_client.h"
#include "ui/aura/focus_manager.h"
#include "ui/aura/root_window.h"
#include "ui/aura/window_property.h"
#include "ui/base/events/event_utils.h"
#include "base/strings/utf_string_conversions.h"
#include "ui/base/ozone/surface_factory_ozone.h"
#include "ui/gfx/insets.h"
#include "ui/linux_ui/linux_ui.h"
#include "ui/native_theme/native_theme.h"
#include "ui/views/corewm/compound_event_filter.h"
#include "ui/views/corewm/corewm_switches.h"
#include "ui/views/corewm/cursor_manager.h"
#include "ui/views/corewm/focus_controller.h"
#include "ui/views/widget/desktop_aura/desktop_activation_client.h"
#include "ui/views/widget/desktop_aura/desktop_dispatcher_client.h"
#include "ui/views/widget/desktop_aura/desktop_focus_rules.h"
#include "ui/views/widget/desktop_aura/desktop_layout_manager.h"
#include "ui/views/widget/desktop_aura/desktop_native_cursor_manager.h"
#include "ui/views/widget/desktop_aura/desktop_native_widget_aura.h"
#include "ui/views/widget/desktop_aura/desktop_screen_position_client.h"

namespace views {

DesktopRootWindowHostWayland* DesktopRootWindowHostWayland::g_current_capture =
    NULL;

DEFINE_WINDOW_PROPERTY_KEY(
    aura::Window*, kViewsWindowForRootWindow, NULL);

DEFINE_WINDOW_PROPERTY_KEY(
    DesktopRootWindowHostWayland*, kHostForRootWindow, NULL);

DesktopRootWindowHostWayland::DesktopRootWindowHostWayland(
    internal::NativeWidgetDelegate* native_widget_delegate,
    DesktopNativeWidgetAura* desktop_native_widget_aura,
    const gfx::Rect& bounds)
    : close_widget_factory_(this),
      native_widget_delegate_(native_widget_delegate),
      desktop_native_widget_aura_(desktop_native_widget_aura) {
}

DesktopRootWindowHostWayland::~DesktopRootWindowHostWayland() {
  root_window_->ClearProperty(kHostForRootWindow);
  if (corewm::UseFocusControllerOnDesktop()) {
    aura::client::SetFocusClient(root_window_, NULL);
    aura::client::SetActivationClient(root_window_, NULL);
  }
}

// static
ui::NativeTheme* DesktopRootWindowHost::GetNativeTheme(aura::Window* window) {
  const ui::LinuxUI* linux_ui = ui::LinuxUI::instance();
  if (linux_ui) {
    ui::NativeTheme* native_theme = linux_ui->GetNativeTheme();
    if (native_theme)
      return native_theme;
  }

  return ui::NativeTheme::instance();
}

////////////////////////////////////////////////////////////////////////////////
// DesktopRootWindowHostWayland, private:

void DesktopRootWindowHostWayland::InitWaylandWindow(
    const Widget::InitParams& params) {
  native_window_ = NULL;
  ui::SurfaceFactoryOzone* surface_factory = ui::SurfaceFactoryOzone::GetInstance();
  ui::SurfaceFactoryOzone::HardwareState displayInitialization = surface_factory->InitializeHardware();
  if (displayInitialization != ui::SurfaceFactoryOzone::INITIALIZED) {
    LOG(ERROR) << "OZONE failed to initialize hardware";
    return;
  }

  window_ = surface_factory->GetAcceleratedWidget();
  native_window_ = (ui::WaylandWindow*)window_;
  surface_factory->AttemptToResizeAcceleratedWidget(window_, params.bounds);
}

void DesktopRootWindowHostWayland::HandleNativeWidgetActivationChanged(
    bool active) {
  native_widget_delegate_->OnNativeWidgetActivationChanged(active);
  native_widget_delegate_->AsWidget()->GetRootView()->SchedulePaint();
}

aura::RootWindow* DesktopRootWindowHostWayland::InitRootWindow(
    const Widget::InitParams& params) {

  aura::RootWindow::CreateParams rw_params(params.bounds);
  rw_params.host = this;
  root_window_ = new aura::RootWindow(rw_params);
  root_window_->Init();
  root_window_->AddChild(content_window_);
  root_window_->SetLayoutManager(new DesktopLayoutManager(root_window_));
  root_window_->SetProperty(kViewsWindowForRootWindow, content_window_);
  root_window_->SetProperty(kHostForRootWindow, this);
  root_window_host_delegate_ = root_window_;

  // If we're given a parent, we need to mark ourselves as transient to another
  // window. Otherwise activation gets screwy.
  gfx::NativeView parent = params.parent;
  if (!params.child && params.parent)
    parent->AddTransientChild(content_window_);

  native_widget_delegate_->OnNativeWidgetCreated(true);

  desktop_native_widget_aura_->CreateCaptureClient(root_window_);

  base::MessagePumpOzone::Current()->AddDispatcherForRootWindow(this);

  if (corewm::UseFocusControllerOnDesktop()) {
    corewm::FocusController* focus_controller =
        new corewm::FocusController(new DesktopFocusRules);
    focus_client_.reset(focus_controller);
    aura::client::SetFocusClient(root_window_, focus_controller);
    aura::client::SetActivationClient(root_window_, focus_controller);
    root_window_->AddPreTargetHandler(focus_controller);
  } else {
    focus_client_.reset(new aura::FocusManager);
    aura::client::SetFocusClient(root_window_, focus_client_.get());
    activation_client_.reset(new DesktopActivationClient(root_window_));
  }

  dispatcher_client_.reset(new DesktopDispatcherClient);
  aura::client::SetDispatcherClient(root_window_,
                                    dispatcher_client_.get());

  // TODO(vignatti): cursor_client_?

  position_client_.reset(new DesktopScreenPositionClient);
  aura::client::SetScreenPositionClient(root_window_,
                                        position_client_.get());

  desktop_native_widget_aura_->InstallInputMethodEventFilter(root_window_);

  // TODO(vignatti): drag_drop_client?

  focus_client_->FocusWindow(content_window_);

  if (native_window_) {
    root_window_host_delegate_->OnHostResized(native_window_->GetBounds().size());
    native_widget_delegate_->AsWidget()->OnNativeWidgetMove();
    // (kalyan) Handle all other cases i.e Menu, parent, top level etc
  }

  return root_window_;
}

bool DesktopRootWindowHostWayland::IsWindowManagerPresent() {
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// DesktopRootWindowHostWayland, DesktopRootWindowHost implementation:

aura::RootWindow* DesktopRootWindowHostWayland::Init(
    aura::Window* content_window,
    const Widget::InitParams& params) {
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
  return InitRootWindow(sanitized_params);
}

void DesktopRootWindowHostWayland::InitFocus(aura::Window* window) {
}

void DesktopRootWindowHostWayland::Close() {
  // TODO(erg): Might need to do additional hiding tasks here.
  native_window_ = NULL;
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
  base::MessagePumpOzone::Current()->RemoveDispatcherForRootWindow(0);
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
  Maximize();
  Show();
}

bool DesktopRootWindowHostWayland::IsVisible() const {
  if (native_window_)
    return native_window_->State() & ui::WaylandWindow::Visible;

  return false;
}

void DesktopRootWindowHostWayland::SetSize(const gfx::Size& size) {
  if (native_window_)
    native_window_->SetBounds(gfx::Rect(0, 0, size.width(), size.height()));
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
  if (native_window_)
    *bounds = native_window_->GetBounds();
  else
    *bounds = gfx::Rect();

  // TODO(erg): This needs a better implementation. For now, we're just pass
  // back the normal state until we keep track of this.
  *show_state = ui::SHOW_STATE_NORMAL;
}

gfx::Rect DesktopRootWindowHostWayland::GetWindowBoundsInScreen() const {
  if (native_window_)
    return native_window_->GetBounds();

  return gfx::Rect();
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
  if (native_window_)
    return native_window_->GetBounds();

  return gfx::Rect();
}

gfx::Rect DesktopRootWindowHostWayland::GetRestoredBounds() const {
  if (native_window_)
    return native_window_->GetBounds();

  return gfx::Rect();
}

gfx::Rect DesktopRootWindowHostWayland::GetWorkAreaBoundsInScreen() const {
  if (native_window_)
    return native_window_->GetBounds();

  return gfx::Rect(0, 0, 10, 10);
}

void DesktopRootWindowHostWayland::SetShape(gfx::NativeRegion native_region) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostWayland::Activate() {
  if (native_window_)
    native_window_->OnActivate();
}

void DesktopRootWindowHostWayland::Deactivate() {
  if (native_window_)
    native_window_->OnDeActivate();
}

bool DesktopRootWindowHostWayland::IsActive() const {
  if (native_window_)
    return native_window_->State() & ui::WaylandWindow::Activated;

  return false;
}

void DesktopRootWindowHostWayland::Maximize() {
  if (native_window_)
    native_window_->OnMaximize();
}

void DesktopRootWindowHostWayland::Minimize() {
  if (native_window_)
    native_window_->OnMinimize();
}

void DesktopRootWindowHostWayland::Restore() {
  if (native_window_)
    native_window_->OnRestore();
}

bool DesktopRootWindowHostWayland::IsMaximized() const {
  if (native_window_)
    return native_window_->State() & ui::WaylandWindow::Maximized;

  return false;
}

bool DesktopRootWindowHostWayland::IsMinimized() const {
  if (native_window_)
    return native_window_->State() & ui::WaylandWindow::Minimized;

  return false;
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

void DesktopRootWindowHostWayland::SetAlwaysOnTop(bool always_on_top) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostWayland::SetWindowTitle(const string16& title) {
  if (native_window_)
    native_window_->SetWindowTitle( UTF16ToUTF8(title).c_str());
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
    Widget::MoveLoopSource source) {
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
  if (native_window_)
    native_window_->SetFullScreen(fullscreen);
}

bool DesktopRootWindowHostWayland::IsFullscreen() const {
  if (native_window_)
    return native_window_->State() & ui::WaylandWindow::FullScreen;

  return false;
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

void DesktopRootWindowHostWayland::OnNativeWidgetFocus() {
  NOTIMPLEMENTED();
}

void DesktopRootWindowHostWayland::OnNativeWidgetBlur() {
  NOTIMPLEMENTED();
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
  if (native_window_) {
    native_window_->OnShow();
    root_window_host_delegate_->OnHostPaint(gfx::Rect(native_window_->GetBounds().size()));
  }
}

void DesktopRootWindowHostWayland::Hide() {
  if (native_window_)
    native_window_->OnHide();
}

void DesktopRootWindowHostWayland::ToggleFullScreen() {
  if (native_window_)
    native_window_->ToggleFullScreen();
}

gfx::Rect DesktopRootWindowHostWayland::GetBounds() const {
  if (native_window_)
  return native_window_->GetBounds();

  return gfx::Rect();
}

void DesktopRootWindowHostWayland::SetBounds(const gfx::Rect& bounds) {
   gfx::Rect currentBounds = native_window_->GetBounds();
  bool origin_changed = currentBounds.origin() != bounds.origin();
  bool size_changed = currentBounds.size() != bounds.size();

  if (origin_changed)
    native_widget_delegate_->AsWidget()->OnNativeWidgetMove();
  if (size_changed)
    root_window_host_delegate_->OnHostResized(bounds.size());
  else
    root_window_host_delegate_->OnHostPaint(gfx::Rect(bounds.size()));

  if (size_changed || origin_changed)
    native_window_->SetBounds(bounds);
}

gfx::Insets DesktopRootWindowHostWayland::GetInsets() const {
  return gfx::Insets();
}

void DesktopRootWindowHostWayland::SetInsets(const gfx::Insets& insets) {
}

gfx::Point DesktopRootWindowHostWayland::GetLocationOnNativeScreen() const {
  if (native_window_)
    return native_window_->GetBounds().origin();

  return gfx::Point();
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

}  // namespace views
