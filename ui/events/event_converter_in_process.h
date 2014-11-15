// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OZONE_UI_EVENTS_EVENT_CONVERTER_IN_PROCESS_H_
#define OZONE_UI_EVENTS_EVENT_CONVERTER_IN_PROCESS_H_

#include <string>

#include "base/memory/scoped_ptr.h"
#include "ozone/ui/events/event_converter_ozone_wayland.h"
#include "ozone/ui/events/keyboard_code_conversion_ozone.h"
#include "ozone/ui/events/keyboard_engine_xkb.h"
#include "ui/events/event.h"
#include "ui/events/platform/platform_event_source.h"

namespace ui {

class EventConverterInProcess : public ui::EventConverterOzoneWayland,
                                public ui::PlatformEventSource {
 public:
  EventConverterInProcess();
  virtual ~EventConverterInProcess();

  void MotionNotify(float x, float y) override;
  void ButtonNotify(unsigned handle,
                    ui::EventType type,
                    ui::EventFlags flags,
                    float x,
                    float y) override;
  void AxisNotify(float x,
                  float y,
                  int xoffset,
                  int yoffset) override;
  void PointerEnter(unsigned handle, float x, float y) override;
  void PointerLeave(unsigned handle, float x, float y) override;
  void KeyNotify(ui::EventType type,
                 unsigned code) override;
  void VirtualKeyNotify(ui::EventType type,
                        uint32_t key,
                        uint32_t modifiers) override;
  void KeyModifiers(uint32_t mods_depressed,
                    uint32_t mods_latched,
                    uint32_t mods_locked,
                    uint32_t group) override;
  void TouchNotify(ui::EventType type,
                   float x,
                   float y,
                   int32_t touch_id,
                   uint32_t time_stamp) override;
  void CloseWidget(unsigned handle) override;

  void OutputSizeChanged(unsigned width, unsigned height) override;
  void WindowResized(unsigned windowhandle,
                     unsigned width,
                     unsigned height) override;
  void WindowUnminimized(unsigned windowhandle) override;
  void WindowDeActivated(unsigned windowhandle) override;
  void WindowActivated(unsigned windowhandle) override;

  void Commit(unsigned handle, const std::string& text) override;
  void PreeditChanged(unsigned handle,
                      const std::string& text,
                      const std::string& commit) override;
  void PreeditEnd() override;
  void PreeditStart() override;
  void InitializeXKB(base::SharedMemoryHandle fd, uint32_t size) override;

  void SetWindowChangeObserver(ui::WindowChangeObserver* observer) override;
  void SetIMEChangeObserver(ui::IMEChangeObserver* observer) override;
  void SetOutputChangeObserver(ui::OutputChangeObserver* observer) override;

 private:
  // PlatformEventSource:
  void OnDispatcherListChanged() override;
  static void NotifyMotion(EventConverterInProcess* data,
                           float x,
                           float y);
  static void NotifyButtonPress(EventConverterInProcess* data,
                                unsigned handle,
                                ui::EventType type,
                                ui::EventFlags flags,
                                float x,
                                float y);
  static void NotifyAxis(EventConverterInProcess* data,
                         float x,
                         float y,
                         int xoffset,
                         int yoffset);
  static void NotifyPointerEnter(EventConverterInProcess* data,
                                 unsigned handle,
                                 float x,
                                 float y);
  static void NotifyPointerLeave(EventConverterInProcess* data,
                                 unsigned handle,
                                 float x,
                                 float y);
  static void NotifyKeyEvent(EventConverterInProcess* data,
                             ui::EventType type,
                             ui::KeyboardCode code,
                             uint16 CharacterCodeFromNativeKey,
                             unsigned modifiers);
  static void NotifyTouchEvent(EventConverterInProcess* data,
                               ui::EventType type,
                               float x,
                               float y,
                               int32_t touch_id,
                               uint32_t time_stamp);
  static void NotifyCloseWidget(EventConverterInProcess* data,
                                unsigned handle);
  static void NotifyOutputSizeChanged(EventConverterInProcess* data,
                                      unsigned width,
                                      unsigned height);
  static void NotifyWindowResized(EventConverterInProcess* data,
                                  unsigned handle,
                                  unsigned width,
                                  unsigned height);
  static void NotifyWindowUnminimized(EventConverterInProcess* data,
                                      unsigned handle);
  static void NotifyWindowDeActivated(EventConverterInProcess* data,
                                      unsigned handle);
  static void NotifyWindowActivated(EventConverterInProcess* data,
                                    unsigned handle);
  static void NotifyCommit(EventConverterInProcess* data, unsigned handle,
                           const std::string& text);
  static void NotifyPreeditChanged(EventConverterInProcess* data,
                                   unsigned handle,
                                   const std::string& text,
                                   const std::string& commit);
  static void NotifyPreeditEnd(EventConverterInProcess* data);
  static void NotifyPreeditStart(EventConverterInProcess* data);

  ui::WindowChangeObserver* observer_;
  ui::IMEChangeObserver* ime_observer_;
  ui::OutputChangeObserver* output_observer_;
  ui::KeyboardEngineXKB* backend_;
  base::Callback<void(void*)> dispatch_callback_;  // NOLINT(readability/
                                                   // function)
  DISALLOW_COPY_AND_ASSIGN(EventConverterInProcess);
};

}  // namespace ui

#endif  // OZONE_UI_EVENTS_EVENT_CONVERTER_IN_PROCESS_H_
