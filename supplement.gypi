# Copyright 2013 The Chromium Authors. All rights reserved.
# Copyright 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables':  {
    'ozone_platform_wayland%': 1,
  },
  'conditions': [
    ['<(ozone_platform_wayland) == 1', {
      'variables':  {
        'external_ozone_platform_deps': [
          '<(DEPTH)/ozone/ozone_impl.gyp:wayland',
        ],
        'external_ozone_events_files': [
          '<(DEPTH)/ozone/ui/events/event_converter_ozone_wayland.h',
          '<(DEPTH)/ozone/ui/events/event_converter_ozone_wayland.cc',
          '<(DEPTH)/ozone/ui/events/event_factory_ozone_wayland.h',
          '<(DEPTH)/ozone/ui/events/event_factory_ozone_wayland.cc',
          '<(DEPTH)/ozone/ui/events/ime_state_change_handler.h',
          '<(DEPTH)/ozone/ui/events/ime_state_change_handler.cc',
          '<(DEPTH)/ozone/ui/events/keyboard_codes_ozone.h',
          '<(DEPTH)/ozone/ui/events/keyboard_code_conversion_ozone.h',
          '<(DEPTH)/ozone/ui/events/keyboard_code_conversion_ozone.cc',
          '<(DEPTH)/ozone/ui/events/output_change_observer.h',
          '<(DEPTH)/ozone/ui/events/window_change_observer.h',
          '<(DEPTH)/ozone/ui/events/window_constants.h',
          '<(DEPTH)/ozone/ui/events/window_state_change_handler.h',
          '<(DEPTH)/ozone/ui/events/window_state_change_handler.cc',
        ],
        'external_ozone_gfx_files': [
          '<(DEPTH)/ozone/ui/gfx/ozone_display.h',
          '<(DEPTH)/ozone/ui/gfx/ozone_display.cc',
          '<(DEPTH)/ozone/ui/gfx/surface_factory_wayland.cc',
          '<(DEPTH)/ozone/ui/gfx/surface_factory_wayland.h',
          '<(DEPTH)/ozone/ui/gfx/vsync_provider_wayland.cc',
          '<(DEPTH)/ozone/ui/gfx/vsync_provider_wayland.h',
        ],
        'external_ozone_platforms': [
          'wayland'
        ],
        'ozone_platform%': 'wayland',
      },
    }],
  ],
}
