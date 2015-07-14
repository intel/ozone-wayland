# Copyright 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'webui',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/ui/base/ui_base.gyp:ui_base',
        '<(DEPTH)/build/linux/system.gyp:pangocairo',
      ],
      'sources': [
        'file_picker_web_dialog.h',
        'select_file_dialog_impl_webui.h',
        'select_file_dialog_impl_webui.cc',
        'ozone_webui.h',
	'ozone_webui.cc',
	'input_method_context_impl_wayland.h',
	'input_method_context_impl_wayland.cc',
      ],
    }
  ]
}
