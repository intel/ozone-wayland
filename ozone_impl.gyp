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
        '../base/base.gyp:base_i18n',
        '../ui/compositor/compositor.gyp:compositor',
        '../ui/gl/gl.gyp:gl',
        '../ui/ui.gyp:ui',
        '../ui/ui.gyp:ui_resources',
      ],
      'include_dirs': [
        '.',
        '../..',
      ],
      'sources': [
        'wayland/egl/egl_window.cc',
        'wayland/egl/egl_window.h',
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
        "wayland/keyboard.cc",
        "wayland/keyboard.h",
        "wayland/egl/loader.h",
        "wayland/egl/loader.cc",
        "wayland/pointer.cc",
        "wayland/pointer.h",
        'wayland/screen.cc',
        'wayland/screen.h',
        'wayland/surface.h',
        'wayland/surface.cc',
        'wayland/shell_surface.h',
        'wayland/shell_surface.cc',
        'wayland/window.cc',
        'wayland/window.h',
      ],
    },
    {
      'target_name': 'wayland',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/ui/ui.gyp:ui',
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
        'impl/ozone_display.cc',
        'impl/ozone_display.h',
        'impl/ipc/child_process_observer.h',
        'impl/ipc/child_process_observer.cc',
        'impl/ipc/display_channel.h',
        'impl/ipc/display_channel.cc',
        'impl/ipc/display_channel_host.h',
        'impl/ipc/display_channel_host.cc',
        'impl/ipc/messages.h',
        'impl/ipc/message_generator.h',
        'impl/ipc/message_generator.cc',
      ],
    },
  ]
}
