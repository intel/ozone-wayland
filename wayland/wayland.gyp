# Copyright 2013 The Chromium Authors. All rights reserved.
# Copyright 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'conditions': [
      ['sysroot!=""', {
        'pkg-config': './pkg-config-wrapper "<(sysroot)" "<(target_arch)"',
      }, {
        'pkg-config': 'pkg-config'
      }],
    ],
  },

  'targets': [
    {
      'target_name': 'wayland_toolkit',
      'type': 'static_library',
      'variables': {
        'WAYLAND_VERSION': '1.2.0',
        'MESA_VERSION': '9.1.3',
        'wayland_packages': [
          'wayland-client >= <(WAYLAND_VERSION)',
          'wayland-cursor >= <(WAYLAND_VERSION)',
          'wayland-egl >= <(MESA_VERSION)',
          'xkbcommon',
        ],
      },
      'cflags': [
        '<!@(<(pkg-config) --cflags <(wayland_packages))',
      ],
      'direct_dependent_settings': {
        'cflags': [
          '<!@(<(pkg-config) --cflags <(wayland_packages))',
        ],
      },
      'link_settings': {
        'ldflags': [
          '<!@(<(pkg-config) --libs-only-L --libs-only-other <(wayland_packages))',
        ],
        'libraries': [
          '<!@(<(pkg-config) --libs-only-l <(wayland_packages))',
        ],
      },
      'dependencies': [
        '../../base/base.gyp:base',
      ],
      'include_dirs': [
        '.',
        '../../..',
      ],
      'sources': [
        'display.cc',
        'display.h',
        'dispatcher.cc',
        'dispatcher.h',
        'global.h',
        'input_device.cc',
        'input_device.h',
        'screen.cc',
        'screen.h',
        'surface.h',
        'surface.cc',
        'shell_surface.h',
        'shell_surface.cc',
        'window.cc',
        'window.h',
        'egl/egl_window.cc',
        'egl/egl_window.h',
        'inputs/cursor.cc',
        'inputs/cursor.h',
        'inputs/input_method_event_filter.cc',
        'inputs/input_method_event_filter.h',
        'inputs/kbd_conversion.cc',
        'inputs/kbd_conversion.h',
        'inputs/keyboard.cc',
        'inputs/keyboard.h',
        'inputs/pointer.cc',
        'inputs/pointer.h',
      ],
    },
  ]
}
