// Copyright 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ozone/content/remote_event_dispatcher.h"

#include <string>

#include "base/bind.h"
#include "content/child/child_process.h"
#include "content/child/child_thread.h"
#include "ozone/content/messages.h"

namespace {

content::ChildThread* GetProcessMainThread() {
  content::ChildProcess* process = content::ChildProcess::current();
  DCHECK(process && process->main_thread());
  return process ? process->main_thread() : NULL;
}

}

namespace content {

RemoteEventDispatcher::RemoteEventDispatcher()
    : EventConverterOzoneWayland() {
}

RemoteEventDispatcher::~RemoteEventDispatcher() {
}

void RemoteEventDispatcher::MotionNotify(float x, float y) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(
      base::Bind(&RemoteEventDispatcher::SendMotionNotify, x, y));
}

void RemoteEventDispatcher::ButtonNotify(unsigned handle,
                                         ui::EventType type,
                                         ui::EventFlags flags,
                                         float x,
                                         float y) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(
      base::Bind(&RemoteEventDispatcher::SendButtonNotify, handle, type, flags,
          x, y));
}

void RemoteEventDispatcher::AxisNotify(float x,
                                       float y,
                                       int xoffset,
                                       int yoffset) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(
      base::Bind(&RemoteEventDispatcher::SendAxisNotify, x, y, xoffset,
          yoffset));
}

void RemoteEventDispatcher::PointerEnter(unsigned handle,
                                         float x,
                                         float y) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(
      base::Bind(&RemoteEventDispatcher::SendPointerEnter, handle, x, y));
}

void RemoteEventDispatcher::PointerLeave(unsigned handle,
                                         float x,
                                         float y) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(
      base::Bind(&RemoteEventDispatcher::SendPointerLeave, handle, x, y));
}

void RemoteEventDispatcher::KeyNotify(ui::EventType state,
                                      unsigned code,
                                      unsigned modifiers) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(
      base::Bind(&RemoteEventDispatcher::SendKeyNotify, state, code,
          modifiers));
}

void RemoteEventDispatcher::TouchNotify(ui::EventType type,
                                        float x,
                                        float y,
                                        int32_t touch_id,
                                        uint32_t time_stamp) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(
      base::Bind(&RemoteEventDispatcher::SendTouchNotify, type,
          x, y, touch_id, time_stamp));
}

void RemoteEventDispatcher::OutputSizeChanged(unsigned width,
                                              unsigned height) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &RemoteEventDispatcher::SendOutputSizeChanged, width, height));
}

void RemoteEventDispatcher::WindowResized(unsigned handle,
                                          unsigned width,
                                          unsigned height) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &RemoteEventDispatcher::SendWindowResized, handle, width, height));
}

void RemoteEventDispatcher::CloseWidget(unsigned handle) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &RemoteEventDispatcher::SendCloseWidget, handle));
}

void RemoteEventDispatcher::Commit(unsigned handle,
                                   const std::string& text) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &RemoteEventDispatcher::SendCommit, handle, text));
}

void RemoteEventDispatcher::PreeditChanged(unsigned handle,
                                           const std::string& text,
                                           const std::string& commit) {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &RemoteEventDispatcher::SendPreeditChanged, handle, text, commit));
}

void RemoteEventDispatcher::PreeditEnd() {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &RemoteEventDispatcher::SendPreeditEnd));
}

void RemoteEventDispatcher::PreeditStart() {
  ui::EventConverterOzoneWayland::PostTaskOnMainLoop(base::Bind(
      &RemoteEventDispatcher::SendPreeditStart));
}

void RemoteEventDispatcher::SendMotionNotify(float x, float y) {
  Send(new WaylandInput_MotionNotify(x, y));
}

void RemoteEventDispatcher::SendButtonNotify(unsigned handle,
                                             ui::EventType type,
                                             ui::EventFlags flags,
                                             float x,
                                             float y) {
  Send(new WaylandInput_ButtonNotify(handle, type, flags, x, y));
}

void RemoteEventDispatcher::SendAxisNotify(float x,
                                           float y,
                                           int xoffset,
                                           int yoffset) {
  Send(new WaylandInput_AxisNotify(x, y, xoffset, yoffset));
}

void RemoteEventDispatcher::SendPointerEnter(unsigned handle,
                                             float x,
                                             float y) {
  Send(new WaylandInput_PointerEnter(handle, x, y));
}

void RemoteEventDispatcher::SendPointerLeave(unsigned handle,
                                             float x,
                                             float y) {
  Send(new WaylandInput_PointerLeave(handle, x, y));
}

void RemoteEventDispatcher::SendKeyNotify(ui::EventType type,
                                          unsigned code,
                                          unsigned modifiers) {
  Send(new WaylandInput_KeyNotify(type, code, modifiers));
}

void RemoteEventDispatcher::SendTouchNotify(ui::EventType type,
                                            float x,
                                            float y,
                                            int32_t touch_id,
                                            uint32_t time_stamp) {
  Send(new WaylandInput_TouchNotify(type, x, y, touch_id, time_stamp));
}

void RemoteEventDispatcher::SendOutputSizeChanged(unsigned width,
                                                  unsigned height) {
  Send(new WaylandInput_OutputSize(width, height));
}

void RemoteEventDispatcher::SendWindowResized(unsigned handle,
                                              unsigned width,
                                              unsigned height) {
  Send(new WaylandWindow_Resized(handle, width, height));
}

void RemoteEventDispatcher::SendCloseWidget(unsigned handle) {
  Send(new WaylandInput_CloseWidget(handle));
}

void RemoteEventDispatcher::SendCommit(unsigned handle,
                                       const std::string& text) {
  Send(new WaylandInput_Commit(handle, text));
}

void RemoteEventDispatcher::SendPreeditChanged(unsigned handle,
                                               const std::string& text,
                                               const std::string& commit) {
  Send(new WaylandInput_PreeditChanged(handle, text, commit));
}

void RemoteEventDispatcher::SendPreeditEnd() {
  Send(new WaylandInput_PreeditEnd());
}

void RemoteEventDispatcher::SendPreeditStart() {
  Send(new WaylandInput_PreeditStart());
}

void RemoteEventDispatcher::Send(IPC::Message* message) {
  ChildThread* thread = GetProcessMainThread();
  thread->Send(message);
}

}  // namespace content
