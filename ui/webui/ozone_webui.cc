// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/webui/ozone_webui.h"

#include <set>

#include "base/command_line.h"
#include "base/debug/leak_annotations.h"
#include "base/environment.h"
#include "base/i18n/rtl.h"
#include "base/logging.h"
#include "base/nix/mime_util_xdg.h"
#include "base/stl_util.h"
#include "base/strings/stringprintf.h"
#include "ozone/platform/ozone_gpu_platform_support_host.h"
#include "ozone/platform/ozone_platform_wayland.h"
#include "ozone/ui/webui/input_method_context_impl_wayland.h"
#include "ozone/ui/webui/select_file_dialog_impl_webui.h"

namespace views {

OzoneWebUI::OzoneWebUI() : host_(NULL) {
}

OzoneWebUI::~OzoneWebUI() {
}

void OzoneWebUI::Initialize() {
  // TODO(kalyan): This is a hack, get rid  of this.
  ui::OzonePlatform* platform = ui::OzonePlatform::GetInstance();
  host_ = static_cast<ui::OzoneGpuPlatformSupportHost*>(
      platform->GetGpuPlatformSupportHost());
}

ui::SelectFileDialog* OzoneWebUI::CreateSelectFileDialog(
    ui::SelectFileDialog::Listener* listener,
    ui::SelectFilePolicy* policy) const {
  return ui::SelectFileDialogImplWebUI::Create(listener, policy);
}

scoped_ptr<ui::LinuxInputMethodContext> OzoneWebUI::CreateInputMethodContext(
      ui::LinuxInputMethodContextDelegate* delegate, bool is_simple) const {
  DCHECK(host_);
  return scoped_ptr<ui::LinuxInputMethodContext>(
           new ui::InputMethodContextImplWayland(delegate, host_));
}

gfx::FontRenderParams OzoneWebUI::GetDefaultFontRenderParams() const {
  NOTIMPLEMENTED();
  return params_;
}

void OzoneWebUI::GetDefaultFontDescription(
    std::string* family_out,
    int* size_pixels_out,
    int* style_out,
    gfx::FontRenderParams* params_out) const {
  NOTIMPLEMENTED();
}

gfx::Image OzoneWebUI::GetThemeImageNamed(int id) const {
  return gfx::Image();
}

bool OzoneWebUI::GetColor(int id, SkColor* color) const {
  return false;
}

bool OzoneWebUI::HasCustomImage(int id) const {
  return false;
}

SkColor OzoneWebUI::GetFocusRingColor() const {
  return SkColorSetRGB(0x4D, 0x90, 0xFE);
}

SkColor OzoneWebUI::GetThumbActiveColor() const {
  return SkColorSetRGB(244, 244, 244);
}

SkColor OzoneWebUI::GetThumbInactiveColor() const {
  return SkColorSetRGB(234, 234, 234);
}

SkColor OzoneWebUI::GetTrackColor() const {
  return SkColorSetRGB(211, 211, 211);
}

SkColor OzoneWebUI::GetActiveSelectionBgColor() const {
  return SkColorSetRGB(0xCB, 0xE4, 0xFA);
}

SkColor OzoneWebUI::GetActiveSelectionFgColor() const {
  return SK_ColorBLACK;
}

SkColor OzoneWebUI::GetInactiveSelectionBgColor() const {
  return SkColorSetRGB(0xEA, 0xEA, 0xEA);
}

SkColor OzoneWebUI::GetInactiveSelectionFgColor() const {
  return SK_ColorBLACK;
}

double OzoneWebUI::GetCursorBlinkInterval() const {
  return 1.0;
}

ui::NativeTheme* OzoneWebUI::GetNativeTheme(aura::Window* window) const {
  return 0;
}

void OzoneWebUI::SetNativeThemeOverride(
      const NativeThemeGetter& callback) {
}

bool OzoneWebUI::GetDefaultUsesSystemTheme() const {
  return false;
}

void OzoneWebUI::SetDownloadCount(int count) const {
}

void OzoneWebUI::SetProgressFraction(float percentage) const {
}

bool OzoneWebUI::IsStatusIconSupported() const {
  return false;
}

scoped_ptr<StatusIconLinux> OzoneWebUI::CreateLinuxStatusIcon(
  const gfx::ImageSkia& image,
  const base::string16& tool_tip) const {
  return scoped_ptr<views::StatusIconLinux>();
}

gfx::Image OzoneWebUI::GetIconForContentType(
  const std::string& content_type, int size) const {
  return gfx::Image();
}

scoped_ptr<Border> OzoneWebUI::CreateNativeBorder(
  views::LabelButton* owning_button,
  scoped_ptr<views::LabelButtonBorder> border) {
  return border.Pass();
}

void OzoneWebUI::AddWindowButtonOrderObserver(
  WindowButtonOrderObserver* observer) {
}

void OzoneWebUI::RemoveWindowButtonOrderObserver(
  WindowButtonOrderObserver* observer) {
}

bool OzoneWebUI::UnityIsRunning() {
  return 0;
}

LinuxUI::NonClientMiddleClickAction
OzoneWebUI::GetNonClientMiddleClickAction() {
  return MIDDLE_CLICK_ACTION_NONE;
}

void OzoneWebUI::NotifyWindowManagerStartupComplete() {
}

bool OzoneWebUI::MatchEvent(const ui::Event& event,
  std::vector<TextEditCommandAuraLinux>* commands) {
  return false;
}

void OzoneWebUI::UpdateDeviceScaleFactor(float scalefactor) {
}

float OzoneWebUI::GetDeviceScaleFactor() const {
  return 0.0;
}

bool OzoneWebUI::GetTint(int id, color_utils::HSL* tint) const {
  return true;
}

}  // namespace views

views::LinuxUI* BuildWebUI() {
  return new views::OzoneWebUI();
}
