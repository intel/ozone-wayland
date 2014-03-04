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
          '<(DEPTH)/ozone/impl/event_converter_ozone_wayland.h',
          '<(DEPTH)/ozone/impl/event_converter_ozone_wayland.cc',
          '<(DEPTH)/ozone/impl/event_factory_ozone_wayland.h',
          '<(DEPTH)/ozone/impl/event_factory_ozone_wayland.cc',
          '<(DEPTH)/ozone/impl/ime_state_change_handler.h',
          '<(DEPTH)/ozone/impl/ime_state_change_handler.cc',
          '<(DEPTH)/ozone/impl/keyboard_codes_ozone.h',
          '<(DEPTH)/ozone/impl/keyboard_code_conversion_ozone.h',
          '<(DEPTH)/ozone/impl/keyboard_code_conversion_ozone.cc',
          '<(DEPTH)/ozone/impl/output_change_observer.h',
          '<(DEPTH)/ozone/impl/window_change_observer.h',
          '<(DEPTH)/ozone/impl/window_constants.h',
          '<(DEPTH)/ozone/impl/window_state_change_handler.h',
          '<(DEPTH)/ozone/impl/window_state_change_handler.cc',
        ],
        'external_ozone_gfx_files': [
          '<(DEPTH)/ozone/impl/ozone_display.h',
          '<(DEPTH)/ozone/impl/ozone_display.cc',
          '<(DEPTH)/ozone/impl/surface_factory_wayland.cc',
          '<(DEPTH)/ozone/impl/surface_factory_wayland.h',
          '<(DEPTH)/ozone/impl/vsync_provider_wayland.cc',
          '<(DEPTH)/ozone/impl/vsync_provider_wayland.h',
        ],
        'external_ozone_views_files': [
          '<(DEPTH)/ozone/impl/desktop_drag_drop_client_wayland.cc',
          '<(DEPTH)/ozone/impl/desktop_drag_drop_client_wayland.h',
          '<(DEPTH)/ozone/impl/desktop_factory_wayland.cc',
          '<(DEPTH)/ozone/impl/desktop_factory_wayland.h',
          '<(DEPTH)/ozone/impl/desktop_screen_wayland.cc',
          '<(DEPTH)/ozone/impl/desktop_screen_wayland.h',
          '<(DEPTH)/ozone/impl/desktop_window_tree_host_wayland.cc',
          '<(DEPTH)/ozone/impl/desktop_window_tree_host_wayland.h',
          '<(DEPTH)/ozone/impl/ozone_util.cc',
          '<(DEPTH)/ozone/impl/ozone_util.h',
          '<(DEPTH)/ozone/impl/window_tree_host_delegate_wayland.cc',
          '<(DEPTH)/ozone/impl/window_tree_host_delegate_wayland.h',
        ],
        'external_ozone_platforms': [
          'wayland'
        ],
        'ozone_platform%': 'wayland',
      },
    }],
  ],
}
