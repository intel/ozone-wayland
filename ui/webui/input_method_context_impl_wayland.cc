// Copyright 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/webui/input_method_context_impl_wayland.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "base/thread_task_runner_handle.h"
#include "ozone/platform/messages.h"
#include "ozone/platform/ozone_gpu_platform_support_host.h"
#include "ui/base/ime/composition_text.h"

namespace ui {

InputMethodContextImplWayland::InputMethodContextImplWayland(
    LinuxInputMethodContextDelegate* delegate,
    OzoneGpuPlatformSupportHost* sender)
    : delegate_(delegate),
      sender_(sender),
      weak_ptr_factory_(this) {
  CHECK(delegate_);
  sender_->RegisterHandler(this);
}

InputMethodContextImplWayland::~InputMethodContextImplWayland() {
  sender_->UnregisterHandler(this);
}

////////////////////////////////////////////////////////////////////////////////
// InputMethodContextImplWayland, ui::LinuxInputMethodContext implementation:
bool InputMethodContextImplWayland::DispatchKeyEvent(
    const KeyEvent& key_event) {
  return false;
}

void InputMethodContextImplWayland::Reset() {
  sender_->Send(new WaylandDisplay_ImeReset());
}

void InputMethodContextImplWayland::Focus() {
}

void InputMethodContextImplWayland::Blur() {
}

void InputMethodContextImplWayland::SetCursorLocation(const gfx::Rect&) {
}

////////////////////////////////////////////////////////////////////////////////
// GpuPlatformSupportHost implementation:
void InputMethodContextImplWayland::OnChannelEstablished(
    int host_id, scoped_refptr<base::SingleThreadTaskRunner> send_runner,
        const base::Callback<void(IPC::Message*)>& send_callback) {
}

void InputMethodContextImplWayland::OnChannelDestroyed(int host_id) {
}

bool InputMethodContextImplWayland::OnMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(InputMethodContextImplWayland, message)
  IPC_MESSAGE_HANDLER(WaylandInput_Commit, Commit)
  IPC_MESSAGE_HANDLER(WaylandInput_PreeditChanged, PreeditChanged)
  IPC_MESSAGE_HANDLER(WaylandInput_PreeditEnd, PreeditEnd)
  IPC_MESSAGE_HANDLER(WaylandInput_PreeditStart, PreeditStart)
  IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void InputMethodContextImplWayland::OnCommit(unsigned handle,
                                             const std::string& text) {
  delegate_->OnCommit(base::string16(base::ASCIIToUTF16(text.c_str())));
}

void InputMethodContextImplWayland::OnPreeditChanged(
    unsigned handle, const std::string& text, const std::string& commit) {
  ui::CompositionText composition_text;
  composition_text.text = base::string16(base::ASCIIToUTF16(text.c_str()));
  delegate_->OnPreeditChanged(composition_text);
}

void InputMethodContextImplWayland::HideInputPanel() {
  sender_->Send(new WaylandDisplay_HideInputPanel());
}

void InputMethodContextImplWayland::ShowInputPanel() {
  sender_->Send(new WaylandDisplay_ShowInputPanel());
}

void InputMethodContextImplWayland::Commit(unsigned handle,
                                           const std::string& text) {
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&InputMethodContextImplWayland::OnCommit,
          weak_ptr_factory_.GetWeakPtr(), handle, text));
}

void InputMethodContextImplWayland::PreeditChanged(unsigned handle,
                                                   const std::string& text,
                                                   const std::string& commit) {
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&InputMethodContextImplWayland::OnPreeditChanged,
          weak_ptr_factory_.GetWeakPtr(), handle, text, commit));
}

void InputMethodContextImplWayland::PreeditEnd() {
}

void InputMethodContextImplWayland::PreeditStart() {
}

}  // namespace ui
