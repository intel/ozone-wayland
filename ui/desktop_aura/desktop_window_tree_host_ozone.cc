// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/desktop_aura/desktop_window_tree_host_ozone.h"

#include <string>

#include "base/bind.h"
#include "ozone/ui/desktop_aura/desktop_drag_drop_client_wayland.h"
#include "ozone/ui/desktop_aura/desktop_screen_wayland.h"
#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ozone/ui/events/window_state_change_handler.h"
#include "ui/aura/client/cursor_client.h"
#include "ui/aura/client/focus_client.h"
#include "ui/aura/window_property.h"
#include "ui/base/dragdrop/os_exchange_data_provider_aura.h"
#include "ui/base/hit_test.h"
#include "ui/base/ime/input_method.h"
#include "ui/base/ime/input_method_auralinux.h"
#include "ui/events/event_utils.h"
#include "ui/events/platform/platform_event_source.h"
#include "ui/gfx/geometry/insets.h"
#include "ui/native_theme/native_theme.h"
#include "ui/ozone/public/ozone_platform.h"
#include "ui/ozone/public/surface_factory_ozone.h"
#include "ui/platform_window/platform_window.h"
#include "ui/platform_window/platform_window_delegate.h"
#include "ui/views/corewm/tooltip_aura.h"
#include "ui/views/ime/input_method.h"
#include "ui/views/linux_ui/linux_ui.h"
#include "ui/views/views_delegate.h"
#include "ui/views/views_export.h"
#include "ui/views/widget/desktop_aura/desktop_dispatcher_client.h"
#include "ui/views/widget/desktop_aura/desktop_native_cursor_manager.h"
#include "ui/views/widget/desktop_aura/desktop_native_widget_aura.h"
#include "ui/views/widget/desktop_aura/desktop_screen_position_client.h"
#include "ui/wm/core/input_method_event_filter.h"
#include "ui/wm/core/window_util.h"
#include "ui/wm/public/window_move_client.h"

DECLARE_WINDOW_PROPERTY_TYPE(views::DesktopWindowTreeHostOzone*);

namespace views {

DesktopWindowTreeHostOzone*
    DesktopWindowTreeHostOzone::g_current_capture = NULL;

DesktopWindowTreeHostOzone*
    DesktopWindowTreeHostOzone::g_current_dispatcher = NULL;

DesktopWindowTreeHostOzone*
    DesktopWindowTreeHostOzone::g_active_window = NULL;

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

  if (!params.parent) {
    g_active_window = this;
    g_current_dispatcher = g_active_window;
    state_ |= Active;
  }
}

scoped_ptr<corewm::Tooltip>
DesktopWindowTreeHostOzone::CreateTooltip() {
   return scoped_ptr<views::corewm::Tooltip>(new views::corewm::TooltipAura);
}

scoped_ptr<aura::client::DragDropClient>
DesktopWindowTreeHostOzone::CreateDragDropClient(
    DesktopNativeCursorManager* cursor_manager) {
  drag_drop_client_ = new DesktopDragDropClientWayland(window());
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

  if (g_active_window == this)
    g_active_window = NULL;

  if (g_current_dispatcher == this)
    g_current_dispatcher = NULL;

  // Actually free our native resources.
  platform_window_->Close();
  window_ = 0;
  if (open_windows().empty())
    CleanUpWindowList();

  ui::PlatformEventSource* event_source =
      ui::PlatformEventSource::GetInstance();
  if (event_source)
    event_source->RemovePlatformEventDispatcher(this);

  desktop_native_widget_aura_->OnHostClosed();
}

aura::WindowTreeHost* DesktopWindowTreeHostOzone::AsWindowTreeHost() {
  return this;
}

void DesktopWindowTreeHostOzone::ShowWindowWithState(
    ui::WindowShowState show_state) {
  if (show_state == ui::SHOW_STATE_NORMAL ||
      show_state == ui::SHOW_STATE_MAXIMIZED ||
      show_state == ui::SHOW_STATE_FULLSCREEN) {
    Activate();
  }

  state_ |= Visible;
  native_widget_delegate_->AsWidget()->SetInitialFocus(show_state);
}

void DesktopWindowTreeHostOzone::ShowMaximizedWithBounds(
    const gfx::Rect& restored_bounds) {
  Maximize();
  previous_bounds_ = restored_bounds;
  Show();
}

bool DesktopWindowTreeHostOzone::IsVisible() const {
  return state_ & Visible;
}

void DesktopWindowTreeHostOzone::SetSize(const gfx::Size& requested_size) {
  gfx::Size size = AdjustSize(requested_size);
  gfx::Rect new_bounds = platform_window_->GetBounds();
  new_bounds.set_size(size);
  platform_window_->SetBounds(new_bounds);
}

void DesktopWindowTreeHostOzone::StackAtTop() {
}

void DesktopWindowTreeHostOzone::CenterWindow(const gfx::Size& size) {
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
  platform_window_->SetBounds(window_bounds);
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
    return previous_bounds_;

  return GetWindowBoundsInScreen();
}

gfx::Rect DesktopWindowTreeHostOzone::GetWorkAreaBoundsInScreen() const {
  // TODO(kalyan): Take into account wm decorations. i.e Dock, panel etc.
  gfx::Screen *screen = gfx::Screen::GetScreenByType(gfx::SCREEN_TYPE_NATIVE);
  if (!screen)
    NOTREACHED() << "Unable to retrieve valid gfx::Screen";

  gfx::Display display = screen->GetPrimaryDisplay();
  return display.bounds();
}

void DesktopWindowTreeHostOzone::SetShape(SkRegion* native_region) {
  // TODO(erg):
  NOTIMPLEMENTED();
}

void DesktopWindowTreeHostOzone::Activate() {
  if (state_ & Active)
    return;

  if (state_ & Visible) {
    OnActivationChanged(true);
  }
}

void DesktopWindowTreeHostOzone::Deactivate() {
  if (!(state_ & Active))
    return;

  ReleaseCapture();
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
  Relayout();
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
  g_active_window = this;

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
  return g_current_capture == this;
}

void DesktopWindowTreeHostOzone::SetAlwaysOnTop(bool always_on_top) {
  NOTIMPLEMENTED();
}

bool DesktopWindowTreeHostOzone::IsAlwaysOnTop() const {
  NOTIMPLEMENTED();
  return false;
}

void DesktopWindowTreeHostOzone::SetVisibleOnAllWorkspaces(
    bool always_visible) {
  NOTIMPLEMENTED();
}

bool DesktopWindowTreeHostOzone::SetWindowTitle(const base::string16& title) {
  if (title.compare(title_)) {
    ui::EventFactoryOzoneWayland::GetInstance()->GetWindowStateChangeHandler()->
        SetWidgetTitle(window_, title);
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
  // TODO(erg):
  NOTIMPLEMENTED();
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
  NOTIMPLEMENTED();
}

void DesktopWindowTreeHostOzone::OnNativeWidgetFocus() {
  native_widget_delegate_->AsWidget()->GetInputMethod()->OnFocus();
}

void DesktopWindowTreeHostOzone::OnNativeWidgetBlur() {
  if (window_)
    native_widget_delegate_->AsWidget()->GetInputMethod()->OnBlur();
}

bool DesktopWindowTreeHostOzone::IsAnimatingClosed() const {
  return false;
}

bool DesktopWindowTreeHostOzone::IsTranslucentWindowOpacitySupported() const {
  return false;
}

void DesktopWindowTreeHostOzone::SizeConstraintsChanged() {
  NOTIMPLEMENTED();
}


////////////////////////////////////////////////////////////////////////////////
// DesktopWindowTreeHostOzone, aura::WindowTreeHost implementation:
ui::EventSource* DesktopWindowTreeHostOzone::GetEventSource() {
  return this;
}

gfx::AcceleratedWidget DesktopWindowTreeHostOzone::GetAcceleratedWidget() {
  return window_;
}

void DesktopWindowTreeHostOzone::Show() {
  if (state_ & Visible)
    return;

  platform_window_->Show();
  ShowWindow();
  native_widget_delegate_->OnNativeWidgetVisibilityChanged(true);
}

void DesktopWindowTreeHostOzone::Hide() {
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
  if (HasCapture())
    return;

  DesktopWindowTreeHostOzone* old_capturer = g_current_capture;
  g_current_capture = this;
  if (old_capturer) {
    old_capturer->OnHostLostWindowCapture();
  }

  g_current_dispatcher = this;
  platform_window_->SetCapture();
}

void DesktopWindowTreeHostOzone::ReleaseCapture() {
  if (g_current_capture != this)
    return;

  platform_window_->ReleaseCapture();
  OnHostLostWindowCapture();
  g_current_capture = NULL;
  g_current_dispatcher = g_active_window;
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
  // TODO(kalyan): Add support for custom cursor.
  ui::EventFactoryOzoneWayland::GetInstance()->GetWindowStateChangeHandler()->
      SetWidgetCursor(cursor.native_type());
}

void DesktopWindowTreeHostOzone::MoveCursorToNative(
    const gfx::Point& location) {
  NOTIMPLEMENTED();
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
}

void DesktopWindowTreeHostOzone::OnDamageRect(const gfx::Rect& damaged_rect) {
  compositor()->ScheduleRedrawRect(damaged_rect);
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

    if (g_active_window && !g_current_capture)
      g_active_window->Deactivate();

    g_active_window = this;
    state_ |= Active;
    g_current_dispatcher = this;
    OnHostActivated();
  } else {
    state_ &= ~Active;
    if (g_active_window == this)
      g_active_window = NULL;
  }

  desktop_native_widget_aura_->HandleActivationChanged(active);
  native_widget_delegate_->AsWidget()->GetRootView()->SchedulePaint();
}

void DesktopWindowTreeHostOzone::OnLostCapture() {
  ReleaseCapture();
}

void DesktopWindowTreeHostOzone::OnAcceleratedWidgetAvailable(
      gfx::AcceleratedWidget widget,
       float device_pixel_ratio) {
  window_ = widget;
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
  SendEventToProcessor(event);
}

////////////////////////////////////////////////////////////////////////////////
// DesktopWindowTreeHostX11, ui::EventSource implementation:
ui::EventProcessor* DesktopWindowTreeHostOzone::GetEventProcessor() {
  return dispatcher();
}

////////////////////////////////////////////////////////////////////////////////
// WindowTreeHostDelegateWayland, ui::PlatformEventDispatcher implementation:
bool DesktopWindowTreeHostOzone::CanDispatchEvent(
    const ui::PlatformEvent& ne) {
  DCHECK(ne);

  if (g_current_dispatcher && g_current_dispatcher == this)
    return true;

  return false;
}

uint32_t DesktopWindowTreeHostOzone::DispatchEvent(
    const ui::PlatformEvent& ne) {
  ui::EventType type = ui::EventTypeFromNative(ne);

  switch (type) {
    case ui::ET_TOUCH_RELEASED: {
      ReleaseCaptureIfNeeded();
      ui::TouchEvent* touchev = static_cast<ui::TouchEvent*>(ne);
      SendEventToProcessor(touchev);
      break;
    }
    case ui::ET_TOUCH_MOVED:
    case ui::ET_TOUCH_PRESSED:
    case ui::ET_TOUCH_CANCELLED: {
      ui::TouchEvent* touchev = static_cast<ui::TouchEvent*>(ne);
      SendEventToProcessor(touchev);
      break;
    }
    case ui::ET_KEY_PRESSED:
    case ui::ET_KEY_RELEASED: {
      SendEventToProcessor(static_cast<ui::KeyEvent*>(ne));
      break;
    }
    case ui::ET_MOUSEWHEEL: {
      ui::MouseWheelEvent* wheelev = static_cast<ui::MouseWheelEvent*>(ne);
      DispatchMouseEvent(wheelev);
      break;
    }
    case ui::ET_MOUSE_RELEASED: {
      ReleaseCaptureIfNeeded();
      ui::MouseEvent* mouseev = static_cast<ui::MouseEvent*>(ne);
      DispatchMouseEvent(mouseev);
      break;
    }
    case ui::ET_MOUSE_MOVED:
    case ui::ET_MOUSE_DRAGGED:
    case ui::ET_MOUSE_PRESSED:
    case ui::ET_MOUSE_ENTERED:
    case ui::ET_MOUSE_EXITED: {
      ui::MouseEvent* mouseev = static_cast<ui::MouseEvent*>(ne);
      DispatchMouseEvent(mouseev);
      break;
    }
    case ui::ET_SCROLL_FLING_START:
    case ui::ET_SCROLL_FLING_CANCEL:
    case ui::ET_SCROLL: {
      SendEventToProcessor(static_cast<ui::ScrollEvent*>(ne));
      break;
    }
    case ui::ET_UMA_DATA:
      break;
    case ui::ET_UNKNOWN:
      break;
    default:
      NOTIMPLEMENTED() << "WindowTreeHostDelegateWayland: unknown event type.";
  }
  return ui::POST_DISPATCH_STOP_PROPAGATION;
}

////////////////////////////////////////////////////////////////////////////////
// DesktopWindowTreeHostOzone, private:

void DesktopWindowTreeHostOzone::InitOzoneWindow(
    const Widget::InitParams& params) {
  const gfx::Rect& bounds = gfx::Rect(params.bounds.origin(),
                                      AdjustSize(params.bounds.size()));
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

  if (!parent_window && g_active_window)
    parent_window = g_active_window->window_;

  platform_window_->InitPlatformWindow(type, parent_window);
  // If we have a delegate which is providing a default window icon, use that
  // icon.
  gfx::ImageSkia* window_icon = ViewsDelegate::views_delegate ?
      ViewsDelegate::views_delegate->GetDefaultWindowIcon() : NULL;
  if (window_icon) {
    SetWindowIcons(gfx::ImageSkia(), *window_icon);
  }

  CreateCompositor(GetAcceleratedWidget());
  if (ui::PlatformEventSource::GetInstance())
    ui::PlatformEventSource::GetInstance()->AddPlatformEventDispatcher(this);
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
}

std::list<gfx::AcceleratedWidget>&
DesktopWindowTreeHostOzone::open_windows() {
  if (!open_windows_)
    open_windows_ = new std::list<gfx::AcceleratedWidget>();

  return *open_windows_;
}

gfx::Size DesktopWindowTreeHostOzone::AdjustSize(
    const gfx::Size& requested_size) {
  std::vector<gfx::Display> displays =
      gfx::Screen::GetScreenByType(gfx::SCREEN_TYPE_NATIVE)->GetAllDisplays();
  // Compare against all monitor sizes. The window manager can move the window
  // to whichever monitor it wants.
  for (size_t i = 0; i < displays.size(); ++i) {
    if (requested_size == displays[i].size()) {
      return gfx::Size(requested_size.width() - 1,
                       requested_size.height() - 1);
    }
  }
  return requested_size;
}

void DesktopWindowTreeHostOzone::DispatchMouseEvent(ui::MouseEvent* event) {
  // In Windows, the native events sent to chrome are separated into client
  // and non-client versions of events, which we record on our LocatedEvent
  // structures. On Desktop Ozone, we emulate the concept of non-client. Before
  // we pass this event to the cross platform event handling framework, we need
  // to make sure it is appropriately marked as non-client if it's in the non
  // client area, or otherwise, we can get into a state where the a window is
  // set as the |mouse_pressed_handler_| in window_event_dispatcher.cc
  // despite the mouse button being released.
  //
  // We can't do this later in the dispatch process because we share that
  // with ash, and ash gets confused about event IS_NON_CLIENT-ness on
  // events, since ash doesn't expect this bit to be set, because it's never
  // been set before. (This works on ash on Windows because none of the mouse
  // events on the ash desktop are clicking in what Windows considers to be a
  // non client area.) Likewise, we won't want to do the following in any
  // WindowTreeHost that hosts ash.
  if (content_window_ && content_window_->delegate()) {
    int flags = event->flags();
    int hit_test_code =
        content_window_->delegate()->GetNonClientComponent(event->location());
    if (hit_test_code != HTCLIENT && hit_test_code != HTNOWHERE)
      flags |= ui::EF_IS_NON_CLIENT;
    event->set_flags(flags);
  }

  SendEventToProcessor(event);
}

void DesktopWindowTreeHostOzone::ReleaseCaptureIfNeeded() const {
  if (g_current_capture && g_current_dispatcher != g_current_capture)
    g_current_capture->ReleaseCapture();
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

DesktopWindowTreeHost* DesktopWindowTreeHost::Create(
    internal::NativeWidgetDelegate* native_widget_delegate,
    DesktopNativeWidgetAura* desktop_native_widget_aura) {
  return new DesktopWindowTreeHostOzone(native_widget_delegate,
                                        desktop_native_widget_aura);
}

}  // namespace views
