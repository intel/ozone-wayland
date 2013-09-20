// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DESKTOP_ROOT_WINDOW_HOST_WAYLAND_H_
#define DESKTOP_ROOT_WINDOW_HOST_WAYLAND_H_

#include "base/basictypes.h"
#include "ui/aura/root_window_host.h"
#include "ui/views/widget/desktop_aura/desktop_root_window_host.h"

namespace aura {
namespace client {
class FocusClient;
class ScreenPositionClient;
}
}

namespace views {
class DesktopDragDropClientAuraOzone;
class DesktopDispatcherClient;
class OzoneDesktopWindowMoveClient;
class OzoneWindowEventFilter;
namespace corewm {
class CursorManager;
}
}

namespace ozonewayland {

class VIEWS_EXPORT DesktopRootWindowHostWayland :
    public views::DesktopRootWindowHost,
    public aura::RootWindowHost,
    public base::MessageLoop::Dispatcher {
 public:
  DesktopRootWindowHostWayland(
      views::internal::NativeWidgetDelegate* native_widget_delegate,
      views::DesktopNativeWidgetAura* desktop_native_widget_aura,
      const gfx::Rect& bounds);
  virtual ~DesktopRootWindowHostWayland();

  // Called by OzoneDesktopHandler to notify us that the native windowing system
  // has changed our activation.
  void HandleNativeWidgetActivationChanged(bool active);

 private:
  enum {
    Uninitialized = 0x00,
    Visible = 0x01, // Window is Visible.
    FullScreen = 0x02,  // Window is in fullscreen mode.
    Maximized = 0x04, // Window is maximized,
    Minimized = 0x08, // Window is minimized.
    Normal = 0x10, // Window is in Normal Mode.
    Active = 0x20, // Window is Active.
  };

  typedef unsigned RootWindowState;
  // Initializes our Ozone surface to draw on. This method performs all
  // initialization related to talking to the Ozone server.
  void InitWaylandWindow(const views::Widget::InitParams& params);

  // Creates an aura::RootWindow to contain the |content_window|, along with
  // all aura client objects that direct behavior.
  aura::RootWindow* InitRootWindow(const views::Widget::InitParams& params);

  // Returns true if there's an X window manager present... in most cases.  Some
  // window managers (notably, ion3) don't implement enough of ICCCM for us to
  // detect that they're there.
  bool IsWindowManagerPresent();

  // Called when another DRWHL takes capture, or when capture is released
  // entirely.
  void OnCaptureReleased();

  // Dispatches a mouse event, taking mouse capture into account. If a
  // different host has capture, we translate the event to its coordinate space
  // and dispatch it to that host instead.
  void DispatchMouseEvent(ui::MouseEvent* event);

  // Overridden from DesktopRootWindowHost:
  virtual aura::RootWindow* Init(aura::Window* content_window,
                                 const views::Widget::InitParams& params) OVERRIDE;
  virtual void InitFocus(aura::Window* window) OVERRIDE;
  virtual void Close() OVERRIDE;
  virtual void CloseNow() OVERRIDE;
  virtual aura::RootWindowHost* AsRootWindowHost() OVERRIDE;
  virtual void ShowWindowWithState(ui::WindowShowState show_state) OVERRIDE;
  virtual void ShowMaximizedWithBounds(
      const gfx::Rect& restored_bounds) OVERRIDE;
  virtual bool IsVisible() const OVERRIDE;
  virtual void SetSize(const gfx::Size& size) OVERRIDE;
  virtual void CenterWindow(const gfx::Size& size) OVERRIDE;
  virtual void GetWindowPlacement(
      gfx::Rect* bounds,
      ui::WindowShowState* show_state) const OVERRIDE;
  virtual gfx::Rect GetWindowBoundsInScreen() const OVERRIDE;
  virtual gfx::Rect GetClientAreaBoundsInScreen() const OVERRIDE;
  virtual gfx::Rect GetRestoredBounds() const OVERRIDE;
  virtual gfx::Rect GetWorkAreaBoundsInScreen() const OVERRIDE;
  virtual void SetShape(gfx::NativeRegion native_region) OVERRIDE;
  virtual void Activate() OVERRIDE;
  virtual void Deactivate() OVERRIDE;
  virtual bool IsActive() const OVERRIDE;
  virtual void Maximize() OVERRIDE;
  virtual void Minimize() OVERRIDE;
  virtual void Restore() OVERRIDE;
  virtual bool IsMaximized() const OVERRIDE;
  virtual bool IsMinimized() const OVERRIDE;
  virtual bool HasCapture() const OVERRIDE;
  virtual void SetAlwaysOnTop(bool always_on_top) OVERRIDE;
  virtual void SetWindowTitle(const string16& title) OVERRIDE;
  virtual void ClearNativeFocus() OVERRIDE;
  virtual views::Widget::MoveLoopResult RunMoveLoop(
      const gfx::Vector2d& drag_offset,
      views::Widget::MoveLoopSource source) OVERRIDE;
  virtual void EndMoveLoop() OVERRIDE;
  virtual void SetVisibilityChangedAnimationsEnabled(bool value) OVERRIDE;
  virtual bool ShouldUseNativeFrame() OVERRIDE;
  virtual void FrameTypeChanged() OVERRIDE;
  virtual views::NonClientFrameView* CreateNonClientFrameView() OVERRIDE;
  virtual void SetFullscreen(bool fullscreen) OVERRIDE;
  virtual bool IsFullscreen() const OVERRIDE;
  virtual void SetOpacity(unsigned char opacity) OVERRIDE;
  virtual void SetWindowIcons(const gfx::ImageSkia& window_icon,
                              const gfx::ImageSkia& app_icon) OVERRIDE;
  virtual void InitModalType(ui::ModalType modal_type) OVERRIDE;
  virtual void FlashFrame(bool flash_frame) OVERRIDE;
  virtual void SetInactiveRenderingDisabled(bool disable_inactive) OVERRIDE;

  // Overridden from aura::RootWindowHost:
  virtual void SetDelegate(aura::RootWindowHostDelegate* delegate) OVERRIDE;
  virtual aura::RootWindow* GetRootWindow() OVERRIDE;
  virtual gfx::AcceleratedWidget GetAcceleratedWidget() OVERRIDE;
  virtual void Show() OVERRIDE;
  virtual void Hide() OVERRIDE;
  virtual void ToggleFullScreen() OVERRIDE;
  virtual gfx::Rect GetBounds() const OVERRIDE;
  virtual void SetBounds(const gfx::Rect& bounds) OVERRIDE;
  virtual gfx::Insets GetInsets() const OVERRIDE;
  virtual void SetInsets(const gfx::Insets& insets) OVERRIDE;
  virtual gfx::Point GetLocationOnNativeScreen() const OVERRIDE;
  virtual void SetCapture() OVERRIDE;
  virtual void ReleaseCapture() OVERRIDE;
  virtual void SetCursor(gfx::NativeCursor cursor) OVERRIDE;
  virtual bool QueryMouseLocation(gfx::Point* location_return) OVERRIDE;
  virtual bool ConfineCursorToRootWindow() OVERRIDE;
  virtual void UnConfineCursor() OVERRIDE;
  virtual void OnCursorVisibilityChanged(bool show) OVERRIDE;
  virtual void MoveCursorTo(const gfx::Point& location) OVERRIDE;
  virtual void SetFocusWhenShown(bool focus_when_shown) OVERRIDE;
  virtual bool GrabSnapshot(
      const gfx::Rect& snapshot_bounds,
      std::vector<unsigned char>* png_representation) OVERRIDE;
  virtual void PostNativeEvent(const base::NativeEvent& native_event) OVERRIDE;
  virtual void OnDeviceScaleFactorChanged(float device_scale_factor) OVERRIDE;
  virtual void PrepareForShutdown() OVERRIDE;
  virtual void OnRootViewLayout() const OVERRIDE;
  virtual void OnNativeWidgetFocus() OVERRIDE;
  virtual void OnNativeWidgetBlur() OVERRIDE;

  // Overridden from DesktopSelectionProviderAuraOzone:
  //virtual void SetDropHandler(
  //    ui::OSExchangeDataProviderAuraOzone* handler) OVERRIDE;

  // Overridden from Dispatcher:
  virtual bool Dispatch(const base::NativeEvent& event) OVERRIDE;

  base::WeakPtrFactory<DesktopRootWindowHostWayland> close_widget_factory_;

  // We are owned by the RootWindow, but we have to have a back pointer to it.
  aura::RootWindow* root_window_;

  // aura:: objects that we own.
  scoped_ptr<aura::client::FocusClient> focus_client_;
  scoped_ptr<views::corewm::CursorManager> cursor_client_;
  scoped_ptr<views::DesktopDispatcherClient> dispatcher_client_;
  scoped_ptr<aura::client::ScreenPositionClient> position_client_;

  gfx::AcceleratedWidget window_;
  views::internal::NativeWidgetDelegate* native_widget_delegate_;

  RootWindowState state_;

  gfx::Rect bounds_;

  aura::RootWindowHostDelegate* root_window_host_delegate_;
  aura::Window* content_window_;

  views::DesktopNativeWidgetAura* desktop_native_widget_aura_;

  // The current root window host that has capture. While X11 has something
  // like Windows SetCapture()/ReleaseCapture(), it is entirely implicit and
  // there are no notifications when this changes. We need to track this so we
  // can notify widgets when they have lost capture, which controls a bunch of
  // things in views like hiding menus.
  static DesktopRootWindowHostWayland* g_current_capture;

  DISALLOW_COPY_AND_ASSIGN(DesktopRootWindowHostWayland);
};

}  // namespace ozonewayland

#endif  // DESKTOP_ROOT_WINDOW_HOST_WAYLAND_H_
