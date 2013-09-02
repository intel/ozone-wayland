# Copyright (c) 2013 The Chromium Authors. All rights reserved.
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
          '<!@(<(pkg-config) --cflags wayland-client wayland-cursor wayland-egl  xkbcommon)',
        ],
      },
      'link_settings': {
        'ldflags': [
          '<!@(<(pkg-config) --libs-only-L --libs-only-other wayland-client wayland-cursor wayland-egl  xkbcommon)',
        ],
        'libraries': [
          '<!@(<(pkg-config) --libs-only-l wayland-client wayland-cursor wayland-egl xkbcommon)',
        ],
      },
      'dependencies': [
        '../base/base.gyp:base',
        '../base/base.gyp:base_i18n',
        '../ui/compositor/compositor.gyp:compositor',
        '../ui/ui.gyp:ui',
        '../ui/ui.gyp:ui_resources',
      ],
      'include_dirs': [
        '.',
        '../..',
      ],
      'sources': [
        'egl/shims.h',
        'egl/egl_window.h',
        'egl/egl_window.cc',
        'wayland/cursor.cc',
        'wayland/cursor.h',
        'wayland/display.cc',
        'wayland/display.h',
        'wayland/dispatcher.cc',
        'wayland/dispatcher.h',
        'wayland/global.h',
        'wayland/input_device.cc',
        'wayland/input_device.h',
        'wayland/input_method_event_filter.cc',
        'wayland/input_method_event_filter.h',
        "wayland/kbd_conversion.cc",
        "wayland/kbd_conversion.h",
        'wayland/pointer.cc',
        'wayland/pointer.h',
        'wayland/keyboard.cc',
        'wayland/keyboard.h',
        'wayland/screen.cc',
        'wayland/screen.h',
        'wayland/shell_surface.cc',
        'wayland/shell_surface.h',
        'wayland/surface.h',
        'wayland/surface.cc',
        'wayland/window.cc',
        'wayland/window.h',
      ],
    },
    {
      'target_name': 'wayland',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/ui/ui.gyp:ui',
        '<(DEPTH)/ui/gl/gl.gyp:gl',
        '<(DEPTH)/skia/skia.gyp:skia',
        'wayland_toolkit'
      ],
      'sources': [
        'impl/desktop_factory_wayland.cc',
        'impl/desktop_factory_wayland.h',
        'impl/desktop_root_window_host_wayland.cc',
        'impl/desktop_root_window_host_wayland.h',
        'impl/desktop_screen_wayland.cc',
        'impl/desktop_screen_wayland.h',
        'impl/event_factory_wayland.cc',
        'impl/event_factory_wayland.h',
        'impl/surface_factory_wayland.cc',
        'impl/surface_factory_wayland.h',
      ],
    },
  ]
}
