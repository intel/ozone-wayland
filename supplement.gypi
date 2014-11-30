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
        'external_ozone_views_files': [
          '<(DEPTH)/ozone/ui/desktop_aura/desktop_drag_drop_client_wayland.cc',
          '<(DEPTH)/ozone/ui/desktop_aura/desktop_drag_drop_client_wayland.h',
          '<(DEPTH)/ozone/ui/desktop_aura/desktop_factory_wayland.cc',
          '<(DEPTH)/ozone/ui/desktop_aura/desktop_factory_wayland.h',
          '<(DEPTH)/ozone/ui/desktop_aura/desktop_screen_wayland.cc',
          '<(DEPTH)/ozone/ui/desktop_aura/desktop_screen_wayland.h',
          '<(DEPTH)/ozone/ui/desktop_aura/desktop_window_tree_host_ozone.cc',
          '<(DEPTH)/ozone/ui/desktop_aura/desktop_window_tree_host_ozone.h',
          '<(DEPTH)/ozone/ui/desktop_aura/ozone_util.cc',
          '<(DEPTH)/ozone/ui/desktop_aura/ozone_util.h',
        ],
        'external_ozone_platforms': [
          'wayland'
        ],
        'ozone_platform%': 'wayland',
      },
    }],
  ],
}
