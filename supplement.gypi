# Copyright 2013 The Chromium Authors. All rights reserved.
# Copyright 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables':  {
    'ozone_platform_wayland%': 1,
    'platform_list_txt_file': '<(SHARED_INTERMEDIATE_DIR)/ui/ozone/platform_list.txt',
    'desktop_factory_ozone_list_cc_file': '<(INTERMEDIATE_DIR)/ui/views/desktop_factory_ozone_list.cc',
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
          '<(DEPTH)/ozone/ui/desktop_aura/desktop_factory_ozone_stubs.cc',
          '<(DEPTH)/ozone/ui/desktop_aura/desktop_factory_ozone_stubs.h',
          '<(DEPTH)/ozone/ui/desktop_aura/desktop_factory_ozone_wayland.cc',
          '<(DEPTH)/ozone/ui/desktop_aura/desktop_factory_ozone_wayland.h',
          '<(DEPTH)/ozone/ui/desktop_aura/desktop_platform_screen.h',
          '<(DEPTH)/ozone/ui/desktop_aura/desktop_screen_wayland.cc',
          '<(DEPTH)/ozone/ui/desktop_aura/desktop_screen_wayland.h',
          '<(DEPTH)/ozone/ui/desktop_aura/desktop_window_tree_host_ozone.cc',
          '<(DEPTH)/ozone/ui/desktop_aura/desktop_window_tree_host_ozone.h',
          '<(DEPTH)/ozone/ui/desktop_aura/ozone_util.cc',
          '<(DEPTH)/ozone/ui/desktop_aura/ozone_util.h',
          '<(desktop_factory_ozone_list_cc_file)',
        ],
        'external_ozone_platforms': [
          'wayland'
        ],
        'ozone_platform%': 'wayland',
      },
    }],
  ],
  'target_defaults': {
    'target_conditions': [
      ['_target_name=="views"', {
        'actions': [
          {
            'action_name': 'generate_constructor_list',
            'variables': {
              'generator_path': '<(DEPTH)/ui/ozone/generate_constructor_list.py',
            },
            'inputs': [
              '<(generator_path)',
              '<(platform_list_txt_file)',
            ],
            'outputs': [
              '<(desktop_factory_ozone_list_cc_file)',
            ],
            'action': [
              'python',
              '<(generator_path)',
              '--platform_list=<(platform_list_txt_file)',
              '--output_cc=<(desktop_factory_ozone_list_cc_file)',
              '--namespace=views',
              '--typename=DesktopFactoryOzone',
              '--include="ui/views/widget/desktop_aura/desktop_factory_ozone.h"',
            ],
          },
        ],
      }],
    ],
  },
}
