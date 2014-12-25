# Copyright 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'sources': [
    'media_ozone_platform_wayland.cc',
    'media_ozone_platform_wayland.h',
    'h264_dpb.cc',
    'h264_dpb.h',
    'va_surface.h',
    'vaapi_h264_decoder.cc',
    'vaapi_h264_decoder.h',
    'vaapi_picture.cc',
    'vaapi_picture.h',
    'vaapi_tfp_picture_wayland.cc',
    'vaapi_tfp_picture_wayland.h',
    'vaapi_video_decode_accelerator.cc',
    'vaapi_video_decode_accelerator.h',
    'vaapi_wrapper.cc',
    'vaapi_wrapper.h',
  ],
  'variables': {
    'extra_header': 'media/va_wayland_stub_header.fragment',
    'sig_files': ['va_wayland.sigs'],
    'generate_stubs_script': '<(DEPTH)/tools/generate_stubs/generate_stubs.py',
    'outfile_type': 'posix_stubs',
    'stubs_filename_root': 'va_stubs',
    'project_path': 'media',
    'intermediate_dir': '<(INTERMEDIATE_DIR)',
    'output_root': '<(SHARED_INTERMEDIATE_DIR)/va',
  },
  'dependencies': [
    '<(DEPTH)/ui/gl/gl.gyp:gl',
    '<(DEPTH)/third_party/libyuv/libyuv.gyp:libyuv',
  ],
  'include_dirs': [
    '<(DEPTH)/third_party/libva',
    '<(DEPTH)/third_party/mesa/src/include',
    '<(output_root)',
    '<(SHARED_INTERMEDIATE_DIR)/ui/gl',
  ],
  'actions': [
    {
      'action_name': 'generate_stubs',
      'inputs': [
        '<(generate_stubs_script)',
        '<(extra_header)',
        '<@(sig_files)',
      ],
      'outputs': [
        '<(intermediate_dir)/<(stubs_filename_root).cc',
        '<(output_root)/<(project_path)/<(stubs_filename_root).h',
      ],
      'action': ['python',
                 '<(generate_stubs_script)',
                 '-i', '<(intermediate_dir)',
                 '-o', '<(output_root)/<(project_path)',
                 '-t', '<(outfile_type)',
                 '-e', '<(extra_header)',
                 '-s', '<(stubs_filename_root)',
                 '-p', '<(project_path)',
                 '<@(_inputs)',
      ],
      'process_outputs_as_sources': 1,
      'message': 'Generating libva stubs for dynamic loading',
    },
 ]
}
