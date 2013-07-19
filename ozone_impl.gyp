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
        '../base/base.gyp:base_i18n',
        '../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../skia/skia.gyp:skia',
        '../ui/compositor/compositor.gyp:compositor',
        '../ui/ui.gyp:ui',
        '../ui/ui.gyp:ui_resources',
      ],
      'include_dirs': [
        '.',
        '../..',
      ],
      'sources': [
        'wayland_delegate.h',
        'wayland_display.cc',
        'wayland_display.h',
        'wayland_input_device.cc',
        'wayland_input_device.h',
        'wayland_input_method_event_filter.cc',
        'wayland_input_method_event_filter.h',
        "wayland_kbd_conversion.cc",
        "wayland_kbd_conversion.h",
        'wayland_screen.cc',
        'wayland_screen.h',
        'wayland_task.cc',
        'wayland_task.h',
        'wayland_window.cc',
        'wayland_window.h',
      ],
    },
    {
      'target_name': 'wayland',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/ui/ui.gyp:ui',
        '<(DEPTH)/ui/gl/gl.gyp:gl',
        'wayland_toolkit'
      ],
      'sources': [
        'event_factory_wayland.cc',
        'event_factory_wayland.h',
        'surface_factory_wayland.cc',
        'surface_factory_wayland.h',
      ],
    },
  ]
}
