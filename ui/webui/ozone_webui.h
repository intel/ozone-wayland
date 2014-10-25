// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_OZONE_WEB_UI_H_
#define CHROME_BROWSER_UI_OZONE_WEB_UI_H_

#include <map>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/observer_list.h"
#include "ui/base/ime/linux/linux_input_method_context.h"
#include "ui/events/linux/text_edit_key_bindings_delegate_auralinux.h"
#include "ui/gfx/color_utils.h"
#include "ui/gfx/font_render_params.h"
#include "ui/gfx/geometry/insets.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/pango_util.h"
#include "ui/views/border.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/button/label_button_border.h"
#include "ui/views/linux_ui/linux_ui.h"
#include "ui/views/window/frame_buttons.h"

class SkBitmap;

namespace gfx {
class Image;
}

using ui::TextEditCommandAuraLinux;

namespace views {
class Border;
class LabelButton;
class View;
class NativeThemeChangeObserver;
class WindowButtonOrderObserver;
// Interface to Wayland desktop features.
//
class OzoneWebUI : public views::LinuxUI {
 public:
  OzoneWebUI();
  virtual ~OzoneWebUI();

  // ui::LinuxInputMethodContextFactory:
  virtual scoped_ptr<ui::LinuxInputMethodContext> CreateInputMethodContext(
      ui::LinuxInputMethodContextDelegate* delegate) const override;

  // gfx::LinuxFontDelegate:
  virtual gfx::FontRenderParams GetDefaultFontRenderParams() const override;
  virtual scoped_ptr<gfx::ScopedPangoFontDescription>
      GetDefaultPangoFontDescription() const override;
  virtual double GetFontDPI() const override;

  // ui::LinuxShellDialog:
  virtual ui::SelectFileDialog* CreateSelectFileDialog(
      ui::SelectFileDialog::Listener* listener,
      ui::SelectFilePolicy* policy) const override;

  // ui::LinuxUI:
  virtual void Initialize() override;

  // These methods are not needed
  virtual gfx::Image GetThemeImageNamed(int id) const override;
  virtual bool GetColor(int id, SkColor* color) const override;
  virtual bool HasCustomImage(int id) const override;
  virtual SkColor GetFocusRingColor() const override;
  virtual SkColor GetThumbActiveColor() const override;
  virtual SkColor GetThumbInactiveColor() const override;
  virtual SkColor GetTrackColor() const override;
  virtual SkColor GetActiveSelectionBgColor() const override;
  virtual SkColor GetActiveSelectionFgColor() const override;
  virtual SkColor GetInactiveSelectionBgColor() const override;
  virtual SkColor GetInactiveSelectionFgColor() const override;
  virtual double GetCursorBlinkInterval() const override;
  virtual ui::NativeTheme* GetNativeTheme(aura::Window* window) const override;
  virtual void SetNativeThemeOverride(
      const NativeThemeGetter& callback) override;
  virtual bool GetDefaultUsesSystemTheme() const override;
  virtual void SetDownloadCount(int count) const override;
  virtual void SetProgressFraction(float percentage) const override;
  virtual bool IsStatusIconSupported() const override;
  virtual scoped_ptr<StatusIconLinux> CreateLinuxStatusIcon(
      const gfx::ImageSkia& image,
      const base::string16& tool_tip) const override;
  virtual gfx::Image GetIconForContentType(
      const std::string& content_type, int size) const override;
  virtual scoped_ptr<Border> CreateNativeBorder(
      views::LabelButton* owning_button,
      scoped_ptr<views::LabelButtonBorder> border) override;
  virtual void AddWindowButtonOrderObserver(
      WindowButtonOrderObserver* observer) override;
  virtual void RemoveWindowButtonOrderObserver(
      WindowButtonOrderObserver* observer) override;
  virtual bool UnityIsRunning() override;
  virtual NonClientMiddleClickAction GetNonClientMiddleClickAction() override;
  virtual void NotifyWindowManagerStartupComplete() override;

  virtual bool MatchEvent(const ui::Event& event,
     std::vector<TextEditCommandAuraLinux>* commands) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(OzoneWebUI);
  gfx::FontRenderParams params_;
};

}  // namespace views

views::LinuxUI* BuildWebUI();

#endif  // CHROME_BROWSER_UI_OZONE_WEB_UI_H_
