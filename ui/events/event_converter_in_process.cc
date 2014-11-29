// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/ui/events/event_converter_in_process.h"

#include "base/bind.h"
#include "ozone/ui/events/event_factory_ozone_wayland.h"
#include "ozone/ui/events/ime_change_observer.h"
#include "ozone/ui/events/output_change_observer.h"
#include "ozone/ui/events/window_change_observer.h"

namespace ui {

const double KAutoRepeatInitialTimer = 1;
const double KAutoRepeatTimer = 0.5;

EventConverterInProcess::EventConverterInProcess()
    : EventConverterOzoneWayland(),
      backend_(NULL) {
}

EventConverterInProcess::~EventConverterInProcess() {
  delete backend_;
}

void EventConverterInProcess::MotionNotify(float x, float y) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyMotion, this, x, y));
}

void EventConverterInProcess::ButtonNotify(unsigned handle,
                                           ui::EventType type,
                                           ui::EventFlags flags,
                                           float x,
                                           float y) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyButtonPress, this, handle, type,
          flags, x, y));
}

void EventConverterInProcess::AxisNotify(float x,
                                         float y,
                                         int xoffset,
                                         int yoffset) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyAxis, this, x, y, xoffset, yoffset));
}

void EventConverterInProcess::PointerEnter(unsigned handle,
                                           float x,
                                           float y) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyPointerEnter, this, handle, x, y));
}

void EventConverterInProcess::PointerLeave(unsigned handle,
                                           float x,
                                           float y) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyPointerLeave, this, handle, x, y));
}

void EventConverterInProcess::KeyNotify(ui::EventType type,
                                        unsigned code) {
  VirtualKeyNotify(type, code, backend_->GetKeyBoardModifiers());
}

void EventConverterInProcess::VirtualKeyNotify(ui::EventType type,
                                               uint32_t key,
                                               uint32_t modifiers) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyKeyEvent, this, type,
          backend_->KeyboardCodeFromNativeKeysym(key),
              backend_->CharacterCodeFromNativeKeySym(key, modifiers),
                  modifiers));
  if (type != ui::ET_KEY_RELEASED) {
    if (timer_.IsRunning())
      return;

    timer_.Start(
        FROM_HERE, base::TimeDelta::FromSeconds(KAutoRepeatInitialTimer), this,
            &EventConverterInProcess::RepeatAutoKey);
  } else if (timer_.IsRunning()) {
      timer_.Stop();
  }
}

void EventConverterInProcess::KeyModifiers(uint32_t mods_depressed,
                                           uint32_t mods_latched,
                                           uint32_t mods_locked,
                                           uint32_t group) {
  backend_->OnKeyModifiers(mods_depressed,
                           mods_latched,
                           mods_locked,
                           group);
}

void EventConverterInProcess::TouchNotify(ui::EventType type,
                                          float x,
                                          float y,
                                          int32_t touch_id,
                                          uint32_t time_stamp) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyTouchEvent, this, type, x, y, touch_id,
          time_stamp));
}

void EventConverterInProcess::CloseWidget(unsigned handle) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyCloseWidget, handle));
}

void EventConverterInProcess::OutputSizeChanged(unsigned width,
                                                unsigned height) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyOutputSizeChanged, width, height));
}

void EventConverterInProcess::WindowResized(unsigned handle,
                                            unsigned width,
                                            unsigned height) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyWindowResized, handle, width,
          height));
}

void EventConverterInProcess::WindowUnminimized(unsigned handle) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyWindowUnminimized, handle));
}

void EventConverterInProcess::WindowDeActivated(unsigned windowhandle) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyWindowDeActivated, windowhandle));
}

void EventConverterInProcess::WindowActivated(unsigned windowhandle) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyWindowActivated, windowhandle));
}

void EventConverterInProcess::Commit(unsigned handle, const std::string& text) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyCommit, handle, text));
}

void EventConverterInProcess::PreeditChanged(unsigned handle,
                                             const std::string& text,
                                             const std::string& commit) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyPreeditChanged, handle, text,
      commit));
}

void EventConverterInProcess::PreeditEnd() {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
        &EventConverterInProcess::NotifyPreeditEnd, this));
}

void EventConverterInProcess::PreeditStart() {
    ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
        &EventConverterInProcess::NotifyPreeditStart, this));
}

void EventConverterInProcess::InitializeXKB(base::SharedMemoryHandle fd,
                                            uint32_t size) {
  if (backend_) {
    delete backend_;
    backend_ = NULL;
  }

  backend_ = new ui::KeyboardEngineXKB();
  backend_->OnKeyboardKeymap(fd.fd, size);
  close(fd.fd);
}

void EventConverterInProcess::OnDispatcherListChanged() {
  if (!loop_)
    loop_ = base::MessageLoop::current();
}

void EventConverterInProcess::RepeatAutoKey() {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &EventConverterInProcess::NotifyKeyEvent, this, ui::ET_KEY_PRESSED,
          backend_->GetLastKeyboardCode(),
              backend_->GetLastCharacterCode(),
                  backend_->GetKeyBoardModifiers()));
  base::TimeDelta delta = base::TimeDelta::FromSeconds(KAutoRepeatTimer);
  if (timer_.GetCurrentDelay() != delta) {
    timer_.Stop();
    timer_.Start(FROM_HERE,
                 delta,
                 this,
                 &EventConverterInProcess::RepeatAutoKey);
  }
}

void EventConverterInProcess::NotifyMotion(EventConverterInProcess* data,
                                           float x,
                                           float y) {
  gfx::Point position(x, y);
  ui::MouseEvent mouseev(ui::ET_MOUSE_MOVED,
                         position,
                         position,
                         0,
                         0);
  data->DispatchEvent(&mouseev);
}

void EventConverterInProcess::NotifyButtonPress(EventConverterInProcess* data,
                                                unsigned handle,
                                                ui::EventType type,
                                                ui::EventFlags flags,
                                                float x,
                                                float y) {
    gfx::Point position(x, y);
    ui::MouseEvent mouseev(type,
                           position,
                           position,
                           flags,
                           flags);
    data->DispatchEvent(&mouseev);

    if (type == ui::ET_MOUSE_RELEASED) {
      ui::WindowChangeObserver* observer =
          ui::EventFactoryOzoneWayland::GetInstance()->
              GetWindowChangeObserver();
      if (observer)
        observer->OnWindowFocused(handle);
    }
}

void EventConverterInProcess::NotifyAxis(EventConverterInProcess* data,
                                         float x,
                                         float y,
                                         int xoffset,
                                         int yoffset) {
  gfx::Point position(x, y);
  ui::MouseEvent mouseev(ui::ET_MOUSEWHEEL, position, position, 0, 0);
  ui::MouseWheelEvent wheelev(mouseev, xoffset, yoffset);

  data->DispatchEvent(&wheelev);
}

void EventConverterInProcess::NotifyPointerEnter(
    EventConverterInProcess* data, unsigned handle, float x, float y) {
  ui::WindowChangeObserver* observer =
      ui::EventFactoryOzoneWayland::GetInstance()->GetWindowChangeObserver();
  if (observer)
    observer->OnWindowEnter(handle);

  gfx::Point position(x, y);
  ui::MouseEvent mouseev(ui::ET_MOUSE_ENTERED, position, position, 0, 0);
  data->DispatchEvent(&mouseev);
}

void EventConverterInProcess::NotifyPointerLeave(
    EventConverterInProcess* data, unsigned handle, float x, float y) {
  ui::WindowChangeObserver* observer =
      ui::EventFactoryOzoneWayland::GetInstance()->GetWindowChangeObserver();
  if (observer)
    observer->OnWindowLeave(handle);

  gfx::Point position(x, y);
  ui::MouseEvent mouseev(ui::ET_MOUSE_EXITED, position, position, 0, 0);
  data->DispatchEvent(&mouseev);
}

void EventConverterInProcess::NotifyKeyEvent(EventConverterInProcess* data,
                                             ui::EventType type,
                                             ui::KeyboardCode code,
                                             uint16 CharacterCodeFromNativeKey,
                                             unsigned modifiers) {
  ui::KeyEvent keyev(type, code, modifiers);
  keyev.set_character(CharacterCodeFromNativeKey);
  data->DispatchEvent(&keyev);
}

void EventConverterInProcess::NotifyTouchEvent(EventConverterInProcess* data,
                                               ui::EventType type,
                                               float x,
                                               float y,
                                               int32_t touch_id,
                                               uint32_t time_stamp) {
  gfx::Point position(x, y);
  base::TimeDelta time_delta = base::TimeDelta::FromMilliseconds(time_stamp);
  ui::TouchEvent touchev(type, position, touch_id, time_delta);
  data->DispatchEvent(&touchev);
}

void EventConverterInProcess::NotifyCloseWidget(unsigned handle) {
  ui::WindowChangeObserver* observer =
      ui::EventFactoryOzoneWayland::GetInstance()->GetWindowChangeObserver();
  if (observer)
    observer->OnWindowClose(handle);
}

void
EventConverterInProcess::NotifyOutputSizeChanged(unsigned width,
                                                 unsigned height) {
  ui::OutputChangeObserver* observer =
      ui::EventFactoryOzoneWayland::GetInstance()->GetOutputChangeObserver();
  if (observer)
    observer->OnOutputSizeChanged(width, height);
}

void
EventConverterInProcess::NotifyWindowResized(unsigned handle,
                                             unsigned width,
                                             unsigned height) {
  ui::WindowChangeObserver* observer =
      ui::EventFactoryOzoneWayland::GetInstance()->GetWindowChangeObserver();
  if (observer)
    observer->OnWindowResized(handle, width, height);
}

void
EventConverterInProcess::NotifyWindowUnminimized(unsigned handle) {
  ui::WindowChangeObserver* observer =
      ui::EventFactoryOzoneWayland::GetInstance()->GetWindowChangeObserver();
  if (observer)
    observer->OnWindowUnminimized(handle);
}

void
EventConverterInProcess::NotifyWindowDeActivated(unsigned handle) {
  ui::WindowChangeObserver* observer =
      ui::EventFactoryOzoneWayland::GetInstance()->GetWindowChangeObserver();
  if (observer)
    observer->OnWindowDeActivated(handle);
}

void
EventConverterInProcess::NotifyWindowActivated(unsigned handle) {
  ui::WindowChangeObserver* observer =
      ui::EventFactoryOzoneWayland::GetInstance()->GetWindowChangeObserver();
  if (observer)
    observer->OnWindowActivated(handle);
}

void
EventConverterInProcess::NotifyCommit(unsigned handle,
                                      const std::string& text) {
  ui::IMEChangeObserver* observer =
      ui::EventFactoryOzoneWayland::GetInstance()->GetIMEChangeObserver();
  if (observer)
    observer->OnCommit(handle, text);
}

void
EventConverterInProcess::NotifyPreeditChanged(unsigned handle,
                                              const std::string& text,
                                              const std::string& commit) {
  ui::IMEChangeObserver* observer =
      ui::EventFactoryOzoneWayland::GetInstance()->GetIMEChangeObserver();
  if (observer)
    observer->OnPreeditChanged(handle, text, commit);
}

void
EventConverterInProcess::NotifyPreeditEnd(EventConverterInProcess* data) {
}

void
EventConverterInProcess::NotifyPreeditStart(EventConverterInProcess* data) {
}

}  // namespace ui
