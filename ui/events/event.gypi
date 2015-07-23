# Copyright 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'dependencies': [
    '<(DEPTH)/ipc/ipc.gyp:ipc',
    '<(DEPTH)/ui/events/events.gyp:events',
  ],
  'sources': [
    'event_converter_ozone_wayland.h',
    'event_factory_ozone_wayland.h',
    'event_factory_ozone_wayland.cc',
    'ime_change_observer.h',
    'ime_state_change_handler.h',
    'keyboard_codes_ozone.h',
    'remote_event_dispatcher.h',
    'remote_event_dispatcher.cc',
    'remote_state_change_handler.h',
    'remote_state_change_handler.cc',
    'window_constants.h',
   ],
}
