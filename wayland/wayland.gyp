# Copyright 2013 The Chromium Authors. All rights reserved.
# Copyright 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'conditions': [
      ['sysroot!=""', {
        'pkg-config': '../../build/linux/pkg-config-wrapper "<(sysroot)" "<(target_arch)"',
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
        'WAYLAND_VERSION': '1.4.0',
        'MESA_VERSION': '9.1.3',
        'wayland_packages': [
          'egl >= <(MESA_VERSION)',
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
        '../..',
        '<(DEPTH)/third_party/khronos',
      ],
      'sources': [
        'display.cc',
        'display.h',
        'display_poll_thread.cc',
        'display_poll_thread.h',
        'ozone_wayland_screen.cc',
        'ozone_wayland_screen.h',
        'input_device.cc',
        'input_device.h',
        'screen.cc',
        'screen.h',
        'window.cc',
        'window.h',
        'egl/egl_window.cc',
        'egl/egl_window.h',
        'egl/surface_ozone_wayland.cc',
        'egl/surface_ozone_wayland.h',
        'input/cursor.cc',
        'input/cursor.h',
        'input/keyboard.cc',
        'input/keyboard.h',
        'input/pointer.cc',
        'input/pointer.h',
        'input/text_input.h',
        'input/text_input.cc',
        'input/text-protocol.c',
        'input/text-client-protocol.h',
        'input/touchscreen.cc',
        'input/touchscreen.h',
        'shell/shell.cc',
        'shell/shell.h',
        'shell/shell_surface.h',
        'shell/shell_surface.cc',
        'shell/wl_shell_surface.cc',
        'shell/wl_shell_surface.h',
        'shell/xdg_shell_surface.cc',
        'shell/xdg_shell_surface.h',
        'shell/xdg-shell-protocol.c',
        'shell/xdg-shell-client-protocol.h',
        'shell/ivi_shell_surface.cc',
        'shell/ivi_shell_surface.h',
        'shell/ivi-application-protocol.c',
        'shell/ivi-application-client-protocol.h',
      ],
    },
  ]
}
