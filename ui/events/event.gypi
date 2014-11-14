# Copyright 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'dependencies': [
    '<(DEPTH)/ui/events/events.gyp:events',
  ],
  'sources': [
    'event_converter_in_process.h',
    'event_converter_in_process.cc',
    'event_converter_ozone_wayland.h',
    'event_converter_ozone_wayland.cc',
    'event_factory_ozone_wayland.h',
    'event_factory_ozone_wayland.cc',
    'ime_state_change_handler.h',
    'ime_state_change_handler.cc',
    'keyboard_codes_ozone.h',
    'keyboard_code_conversion_ozone.h',
    'keyboard_code_conversion_ozone.cc',
    'keyboard_engine_xkb.h',
    'keyboard_engine_xkb.cc',
    'output_change_observer.h',
    'remote_event_dispatcher.h',
    'remote_event_dispatcher.cc',
    'remote_state_change_handler.h',
    'remote_state_change_handler.cc',
    'window_change_observer.h',
    'window_constants.h',
    'window_state_change_handler.h',
    'window_state_change_handler.cc',
   ],
}
