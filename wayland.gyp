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
      'cflags': [
        '<!@(<(pkg-config) --cflags wayland-client wayland-cursor wayland-egl xkbcommon)',
      ],
      'direct_dependent_settings': {
        'cflags': [
          '<!@(<(pkg-config) --cflags wayland-client wayland-cursor wayland-egl xkbcommon)',
        ],
      },
      'link_settings': {
        'ldflags': [
          '<!@(<(pkg-config) --libs-only-L --libs-only-other wayland-client wayland-cursor wayland-egl xkbcommon)',
        ],
        'libraries': [
          '<!@(<(pkg-config) --libs-only-l wayland-client wayland-cursor wayland-egl xkbcommon)',
        ],
      },
      'dependencies': [
        '../base/base.gyp:base',
        '../ui/gl/gl.gyp:gl',
      ],
      'include_dirs': [
        '.',
        '../..',
      ],
      'sources': [
        'wayland/display.cc',
        'wayland/display.h',
        'wayland/dispatcher.cc',
        'wayland/dispatcher.h',
        'wayland/global.h',
        'wayland/input_device.cc',
        'wayland/input_device.h',
        'wayland/screen.cc',
        'wayland/screen.h',
        'wayland/surface.h',
        'wayland/surface.cc',
        'wayland/shell_surface.h',
        'wayland/shell_surface.cc',
        'wayland/window.cc',
        'wayland/window.h',
        'wayland/egl/egl_window.cc',
        'wayland/egl/egl_window.h',
        "wayland/egl/loader.h",
        "wayland/egl/loader.cc",
        'wayland/inputs/cursor.cc',
        'wayland/inputs/cursor.h',
        'wayland/inputs/input_method_event_filter.cc',
        'wayland/inputs/input_method_event_filter.h',
        "wayland/inputs/kbd_conversion.cc",
        "wayland/inputs/kbd_conversion.h",
        "wayland/inputs/keyboard.cc",
        "wayland/inputs/keyboard.h",
        "wayland/inputs/pointer.cc",
        "wayland/inputs/pointer.h",
      ],
    },
  ]
}
