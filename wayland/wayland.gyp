# Copyright 2013 The Chromium Authors. All rights reserved.
# Copyright 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'variables': {
      'enable_drm_support%': 0,
    },
    'enable_drm_support%': '<(enable_drm_support)',
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
      'defines': [
        'OZONE_WAYLAND_IMPLEMENTATION',
      ],
      'cflags': [
        '<!@(<(pkg-config) --cflags <(wayland_packages))',
      ],
      'direct_dependent_settings': {
        'cflags': [
          '<!@(<(pkg-config) --cflags <(wayland_packages))',
          '<!@(<(pkg-config) --cflags gbm)',
        ],
      },
      'link_settings': {
        'ldflags': [
          '<!@(<(pkg-config) --libs-only-L --libs-only-other <(wayland_packages))',
          '<!@(<(pkg-config) --libs-only-L --libs-only-other gbm)',
        ],
        'libraries': [
          '<!@(<(pkg-config) --libs-only-l <(wayland_packages))',
          '<!@(<(pkg-config) --libs-only-l gbm)',
        ],
      },
      'dependencies': [
        '../../base/base.gyp:base',
	'../../skia/skia.gyp:skia',
        '<(DEPTH)/ui/gfx/ipc/gfx_ipc.gyp:gfx_ipc',
        '../../ui/ozone/ozone.gyp:ozone_base',
      ],
      'include_dirs': [
        '../..',
        '<(DEPTH)/third_party/khronos',
      ],
      'conditions': [
        ['<(enable_drm_support)==1', {
          'defines': [
          'ENABLE_DRM_SUPPORT',
          ],
          'sources': [
            'egl/wayland_pixmap.cc',
            'egl/wayland_pixmap.h',
            'protocol/wayland-drm-protocol.cc',
            'protocol/wayland-drm-protocol.h',
          ],
        }],
      ],
      'sources': [
        'data_device.cc',
        'data_device.h',
        'data_offer.cc',
        'data_offer.h',
        'display.cc',
        'display.h',
        'display_poll_thread.cc',
        'display_poll_thread.h',
        'ozone_wayland_screen.cc',
        'ozone_wayland_screen.h',
        'screen.cc',
        'screen.h',
        'seat.cc',
        'seat.h',
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
        'input/touchscreen.cc',
        'input/touchscreen.h',
        'protocol/text-protocol.c',
        'protocol/text-client-protocol.h',
        'protocol/ivi-application-protocol.c',
        'protocol/ivi-application-client-protocol.h',
        'protocol/xdg-shell-protocol.c',
        'protocol/xdg-shell-client-protocol.h',
        'shell/shell.cc',
        'shell/shell.h',
        'shell/shell_surface.h',
        'shell/shell_surface.cc',
        'shell/wl_shell_surface.cc',
        'shell/wl_shell_surface.h',
        'shell/xdg_shell_surface.cc',
        'shell/xdg_shell_surface.h',
        'shell/ivi_shell_surface.cc',
        'shell/ivi_shell_surface.h',
      ],
    },
  ]
}
