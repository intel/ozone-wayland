// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_IMPL_DESKTOP_AURA_DESKTOP_WINDOW_TREE_HOST_WAYLAND_H_
#define OZONE_IMPL_DESKTOP_AURA_DESKTOP_WINDOW_TREE_HOST_WAYLAND_H_

#include <set>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "ui/aura/window_tree_host.h"
#include "ui/platform_window/platform_window_delegate.h"
#include "ui/views/widget/desktop_aura/desktop_window_tree_host.h"

namespace ui {
class PlatformWindow;
}

namespace views {

namespace corewm {
class Tooltip;
}

class DesktopDragDropClientWayland;
class WindowTreeHostDelegateWayland;

class VIEWS_EXPORT DesktopWindowTreeHostWayland
    : public DesktopWindowTreeHost,
      public aura::WindowTreeHost,
      public ui::PlatformWindowDelegate
{
 public:
  DesktopWindowTreeHostWayland(
      internal::NativeWidgetDelegate* native_widget_delegate,
      DesktopNativeWidgetAura* desktop_native_widget_aura);
  virtual ~DesktopWindowTreeHostWayland();

  // Accepts a opaque handle widget and returns associated
  // DesktopWindowTreeHostWayland.
  static DesktopWindowTreeHostWayland* GetHostForAcceleratedWidget(
      gfx::AcceleratedWidget widget);

  // Get all open top-level windows. This includes windows that may not be
  // visible. This list is sorted in their stacking order, i.e. the first window
  // is the topmost window.
  static const std::vector<aura::Window*>& GetAllOpenWindows();

  // Deallocates the internal list of open windows.
  static void CleanUpWindowList();

  // Accepts a opaque handle widget and returns associated aura::Window.
  static aura::Window* GetContentWindowForAcceleratedWidget(
      gfx::AcceleratedWidget widget);

  // Returns window bounds. This is used by Screen to determine if a point
  // belongs to a particular window.
  gfx::Rect GetBoundsInScreen() const;

 private:
  enum {
    Uninitialized = 0x00,
    Visible = 0x01,  // Window is Visible.
    FullScreen = 0x02,  // Window is in fullscreen mode.
    Maximized = 0x04,  // Window is maximized,
    Minimized = 0x08,  // Window is minimized.
    Normal = 0x10,  // Window is in Normal Mode.
    Active = 0x20  // Window is Active.
  };

  typedef unsigned RootWindowState;

  // Initializes our Ozone surface to draw on. This method performs all
  // initialization related to talking to the Ozone server.
  void InitWaylandWindow(const views::Widget::InitParams& params);

  // Called when another DRWHL takes capture, or when capture is released
  // entirely.
  void OnCaptureReleased();

  // ui::PlatformWindowDelegate:
  // TODO(kalyan) Provide appropriate implementations.
  virtual void OnBoundsChanged(const gfx::Rect&) OVERRIDE {}
  virtual void OnBoundChanged(const gfx::Rect& old_bounds,
                              const gfx::Rect& new_bounds) OVERRIDE;
  virtual void OnDamageRect(const gfx::Rect& damaged_region) OVERRIDE{}
  virtual void DispatchEvent(ui::Event* event) OVERRIDE{}
  virtual void OnCloseRequest() OVERRIDE;
  virtual void OnClosed() OVERRIDE{}
  virtual void OnWindowStateChanged(ui::PlatformWindowState new_state) OVERRIDE{}
  virtual void OnLostCapture() OVERRIDE;
  virtual void OnAcceleratedWidgetAvailable(
      gfx::AcceleratedWidget widget) OVERRIDE;
  virtual void OnActivationChanged(bool active) OVERRIDE;

  // Overridden from DesktopWindowTreeHost:
  virtual void Init(
      aura::Window* content_window,
      const views::Widget::InitParams& params) OVERRIDE;
  virtual void OnNativeWidgetCreated(
      const views::Widget::InitParams& params) OVERRIDE;
  virtual scoped_ptr<views::corewm::Tooltip> CreateTooltip() OVERRIDE;
  virtual scoped_ptr<aura::client::DragDropClient> CreateDragDropClient(
      views::DesktopNativeCursorManager* cursor_manager) OVERRIDE;
  virtual void Close() OVERRIDE;
  virtual void CloseNow() OVERRIDE;
  virtual aura::WindowTreeHost* AsWindowTreeHost() OVERRIDE;
  virtual void ShowWindowWithState(ui::WindowShowState show_state) OVERRIDE;
  virtual void ShowMaximizedWithBounds(
      const gfx::Rect& restored_bounds) OVERRIDE;
  virtual bool IsVisible() const OVERRIDE;
  virtual void SetSize(const gfx::Size& size) OVERRIDE;
  virtual void StackAtTop() OVERRIDE;
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
  virtual bool IsAlwaysOnTop() const OVERRIDE;
  virtual void SetVisibleOnAllWorkspaces(bool always_visible) OVERRIDE;
  virtual void SetAlwaysOnTop(bool always_on_top) OVERRIDE;
  virtual bool SetWindowTitle(const base::string16& title) OVERRIDE;
  virtual void ClearNativeFocus() OVERRIDE;
  virtual views::Widget::MoveLoopResult RunMoveLoop(
      const gfx::Vector2d& drag_offset,
      views::Widget::MoveLoopSource source,
      views::Widget::MoveLoopEscapeBehavior escape_behavior) OVERRIDE;
  virtual void EndMoveLoop() OVERRIDE;
  virtual void SetVisibilityChangedAnimationsEnabled(bool value) OVERRIDE;
  virtual bool ShouldUseNativeFrame() const OVERRIDE;
  virtual bool ShouldWindowContentsBeTransparent() const OVERRIDE;
  virtual void FrameTypeChanged() OVERRIDE;
  virtual void SetFullscreen(bool fullscreen) OVERRIDE;
  virtual bool IsFullscreen() const OVERRIDE;
  virtual void SetOpacity(unsigned char opacity) OVERRIDE;
  virtual void SetWindowIcons(const gfx::ImageSkia& window_icon,
                              const gfx::ImageSkia& app_icon) OVERRIDE;
  virtual void InitModalType(ui::ModalType modal_type) OVERRIDE;
  virtual void FlashFrame(bool flash_frame) OVERRIDE;
  virtual void OnRootViewLayout() OVERRIDE;
  virtual void OnNativeWidgetFocus() OVERRIDE;
  virtual void OnNativeWidgetBlur() OVERRIDE;
  virtual bool IsAnimatingClosed() const OVERRIDE;
  virtual bool IsTranslucentWindowOpacitySupported() const OVERRIDE;
  virtual void SizeConstraintsChanged() OVERRIDE;

  // Overridden from aura::WindowTreeHost:
  virtual ui::EventSource* GetEventSource() OVERRIDE;
  virtual gfx::AcceleratedWidget GetAcceleratedWidget() OVERRIDE;
  virtual void Show() OVERRIDE;
  virtual void Hide() OVERRIDE;
  virtual gfx::Rect GetBounds() const OVERRIDE;
  virtual void SetBounds(const gfx::Rect& bounds) OVERRIDE;
  virtual gfx::Point GetLocationOnNativeScreen() const OVERRIDE;
  virtual void SetCapture() OVERRIDE;
  virtual void ReleaseCapture() OVERRIDE;
  virtual void SetCursorNative(gfx::NativeCursor cursor) OVERRIDE;
  virtual void OnCursorVisibilityChangedNative(bool show) OVERRIDE;
  virtual void MoveCursorToNative(const gfx::Point& location) OVERRIDE;
  virtual void PostNativeEvent(const base::NativeEvent& native_event) OVERRIDE;

  void HandleWindowUnminimized();
  void Relayout();

  static std::list<gfx::AcceleratedWidget>& open_windows();

  RootWindowState state_;

  // Original bounds of DRWH.
  gfx::Rect previous_bounds_;
  gfx::Rect previous_maximize_bounds_;
  gfx::AcceleratedWidget window_;
  base::string16 title_;

  base::WeakPtrFactory<DesktopWindowTreeHostWayland> close_widget_factory_;

  // Owned by DesktopNativeWidgetAura.
  DesktopDragDropClientWayland* drag_drop_client_;
  views::internal::NativeWidgetDelegate* native_widget_delegate_;
  aura::Window* content_window_;

  views::DesktopNativeWidgetAura* desktop_native_widget_aura_;
  // We can optionally have a parent which can order us to close, or own
  // children who we're responsible for closing when we CloseNow().
  DesktopWindowTreeHostWayland* window_parent_;
  std::set<DesktopWindowTreeHostWayland*> window_children_;

  // Platform-specific part of this DesktopWindowTreeHost.
  scoped_ptr<ui::PlatformWindow> platform_window_;

  // A list of all (top-level) windows that have been created but not yet
  // destroyed.
  static std::list<gfx::AcceleratedWidget>* open_windows_;
  // List of all open aura::Window.
  static std::vector<aura::Window*>* aura_windows_;
  static DesktopWindowTreeHostWayland* current_capture_;
  friend class WindowTreeHostDelegateWayland;
  DISALLOW_COPY_AND_ASSIGN(DesktopWindowTreeHostWayland);
};

}  // namespace views

#endif  // OZONE_IMPL_DESKTOP_AURA_DESKTOP_WINDOW_TREE_HOST_WAYLAND_H_
