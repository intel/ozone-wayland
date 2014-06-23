# Copyright 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'dependencies': [
    '<(DEPTH)/ui/base/ui_base.gyp:ui_base',
    '<(DEPTH)/build/linux/system.gyp:pangocairo',
    '../ui/accessibility/accessibility.gyp:accessibility'
  ],
  'sources': [
    'file_picker_web_dialog.h',
    'select_file_dialog_impl_webui.h',
    'select_file_dialog_impl_webui.cc',
    'ozone_webui.h',
    'ozone_webui.cc'
  ],
}
