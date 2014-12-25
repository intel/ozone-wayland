// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vaapi_wrapper.h"

#include <dlfcn.h>
// XXX
#include <wayland-client.h>

#include "base/bind.h"
#include "base/logging.h"
#include "base/numerics/safe_conversions.h"
// Auto-generated for dlopen libva libraries
#include "media/media/va_stubs.h"

#include "third_party/libva/va/wayland/va_wayland.h"
#include "third_party/libva/va/va_drmcommon.h"

using media_media::kModuleVa_wayland;
using media_media::InitializeStubs;
using media_media::StubPathMap;

static const base::FilePath::CharType kVaLib[] =
    FILE_PATH_LITERAL("/home/kalyan/install/lib/libva-wayland.so.1.3400.0");

static const char kVaLockBufferSymbol[] = "vaLockBuffer";
static const char kVaUnlockBufferSymbol[] = "vaUnlockBuffer";

#define LOG_VA_ERROR_AND_REPORT(va_error, err_msg)         \
  do {                                                     \
    DVLOG(1) << err_msg                                    \
             << " VA error: " << vaErrorStr(va_error);     \
    report_error_to_uma_cb_.Run();                         \
  } while (0)

#define VA_LOG_ON_ERROR(va_error, err_msg)                 \
  do {                                                     \
    if ((va_error) != VA_STATUS_SUCCESS)                   \
      LOG_VA_ERROR_AND_REPORT(va_error, err_msg);          \
  } while (0)

#define VA_SUCCESS_OR_RETURN(va_error, err_msg, ret)       \
  do {                                                     \
    if ((va_error) != VA_STATUS_SUCCESS) {                 \
      LOG_VA_ERROR_AND_REPORT(va_error, err_msg);          \
      return (ret);                                        \
    }                                                      \
  } while (0)

namespace media {

// Maps Profile enum values to VaProfile values.
static VAProfile ProfileToVAProfile(
    media::VideoCodecProfile profile,
    const std::vector<VAProfile>& supported_profiles) {

  VAProfile va_profile = VAProfileNone;

  switch (profile) {
    case media::H264PROFILE_BASELINE:
      va_profile = VAProfileH264Baseline;
      break;
    case media::H264PROFILE_MAIN:
      va_profile = VAProfileH264Main;
      break;
    // TODO(posciak): See if we can/want support other variants
    // of media::H264PROFILE_HIGH*.
    case media::H264PROFILE_HIGH:
      va_profile = VAProfileH264High;
      break;
    default:
      break;
  }

  bool supported = std::find(supported_profiles.begin(),
                             supported_profiles.end(),
                             va_profile) != supported_profiles.end();

  if (!supported && va_profile == VAProfileH264Baseline) {
    // crbug.com/345569: media::ProfileIDToVideoCodecProfile() currently strips
    // the information whether the profile is constrained or not, so we have no
    // way to know here. Try for baseline first, but if it is not supported,
    // try constrained baseline and hope this is what it actually is
    // (which in practice is true for a great majority of cases).
    if (std::find(supported_profiles.begin(),
                  supported_profiles.end(),
                  VAProfileH264ConstrainedBaseline) !=
        supported_profiles.end()) {
      va_profile = VAProfileH264ConstrainedBaseline;
      DVLOG(1) << "Falling back to constrained baseline profile.";
    }
  }

  return va_profile;
}

VASurface::VASurface(VASurfaceID va_surface_id, const ReleaseCB& release_cb)
    : va_surface_id_(va_surface_id),
      release_cb_(release_cb) {
  DCHECK(!release_cb_.is_null());
}

VASurface::~VASurface() {
  release_cb_.Run(va_surface_id_);
}

VaapiWrapper::VaapiWrapper()
    : va_display_(NULL),
      va_config_id_(VA_INVALID_ID),
      va_context_id_(VA_INVALID_ID) {
}

VaapiWrapper::~VaapiWrapper() {
  DestroyPendingBuffers();
  DestroySurfaces();
  Deinitialize();
}

scoped_ptr<VaapiWrapper> VaapiWrapper::Create(
    media::VideoCodecProfile profile,
    void* display,
    const base::Closure& report_error_to_uma_cb) {
  scoped_ptr<VaapiWrapper> vaapi_wrapper(new VaapiWrapper());

  if (!vaapi_wrapper->Initialize(profile, display, report_error_to_uma_cb))
    vaapi_wrapper.reset();

  return vaapi_wrapper.Pass();
}

void VaapiWrapper::TryToSetVADisplayAttributeToLocalGPU() {
  VADisplayAttribute item = {VADisplayAttribRenderMode,
                             1,  // At least support '_LOCAL_OVERLAY'.
                             -1,  // The maximum possible support 'ALL'.
                             VA_RENDER_MODE_LOCAL_GPU,
                             VA_DISPLAY_ATTRIB_SETTABLE};

  VAStatus va_res = vaSetDisplayAttributes(va_display_, &item, 1);
  if (va_res != VA_STATUS_SUCCESS)
    DVLOG(2) << "vaSetDisplayAttributes unsupported, ignoring by default.";
}

bool VaapiWrapper::Initialize(media::VideoCodecProfile profile,
                              void* display,
                              const base::Closure& report_error_to_uma_cb) {
  static bool vaapi_functions_initialized = PostSandboxInitialization();
  if (!vaapi_functions_initialized) {
    DVLOG(1) << "Failed to initialize VAAPI libs";
    return false;
  }

  report_error_to_uma_cb_ = report_error_to_uma_cb;

  base::AutoLock auto_lock(va_lock_);

  va_display_ = vaGetDisplayWl(static_cast<wl_display *>(display));
  if (!vaDisplayIsValid(va_display_)) {
    DVLOG(1) << "Could not get a valid VA display";
    return false;
  }

  VAStatus va_res = vaInitialize(va_display_, &major_version_, &minor_version_);
  VA_SUCCESS_OR_RETURN(va_res, "vaInitialize failed", false);
  DVLOG(1) << "VAAPI version: " << major_version_ << "." << minor_version_;

  if (VAAPIVersionLessThan(0, 34)) {
    DVLOG(1) << "VAAPI version < 0.34 is not supported.";
    return false;
  }

  // Query the driver for supported profiles.
  int max_profiles = vaMaxNumProfiles(va_display_);
  std::vector<VAProfile> supported_profiles(
      base::checked_cast<size_t>(max_profiles));

  int num_supported_profiles;
  va_res = vaQueryConfigProfiles(
      va_display_, &supported_profiles[0], &num_supported_profiles);
  VA_SUCCESS_OR_RETURN(va_res, "vaQueryConfigProfiles failed", false);
  if (num_supported_profiles < 0 || num_supported_profiles > max_profiles) {
    DVLOG(1) << "vaQueryConfigProfiles returned: " << num_supported_profiles;
    return false;
  }

  supported_profiles.resize(base::checked_cast<size_t>(num_supported_profiles));

  VAProfile va_profile = ProfileToVAProfile(profile, supported_profiles);
  if (va_profile == VAProfileNone) {
    DVLOG(1) << "Unsupported profile";
    return false;
  }

  VAConfigAttrib attrib = {VAConfigAttribRTFormat, 0};
  const VAEntrypoint kEntrypoint = VAEntrypointVLD;
  va_res = vaGetConfigAttributes(va_display_, va_profile, kEntrypoint,
                                 &attrib, 1);
  VA_SUCCESS_OR_RETURN(va_res, "vaGetConfigAttributes failed", false);

  if (!(attrib.value & VA_RT_FORMAT_YUV420)) {
    DVLOG(1) << "YUV420 not supported by this VAAPI implementation";
    return false;
  }

  TryToSetVADisplayAttributeToLocalGPU();

  va_res = vaCreateConfig(va_display_, va_profile, kEntrypoint,
                          &attrib, 1, &va_config_id_);
  VA_SUCCESS_OR_RETURN(va_res, "vaCreateConfig failed", false);

  return true;
}

void VaapiWrapper::Deinitialize() {
  base::AutoLock auto_lock(va_lock_);

  if (va_config_id_ != VA_INVALID_ID) {
    VAStatus va_res = vaDestroyConfig(va_display_, va_config_id_);
    VA_LOG_ON_ERROR(va_res, "vaDestroyConfig failed");
  }

  if (va_display_) {
    VAStatus va_res = vaTerminate(va_display_);
    VA_LOG_ON_ERROR(va_res, "vaTerminate failed");
  }

  va_config_id_ = VA_INVALID_ID;
  va_display_ = NULL;
}

bool VaapiWrapper::VAAPIVersionLessThan(int major, int minor) {
  return (major_version_ < major) ||
      (major_version_ == major && minor_version_ < minor);
}

bool VaapiWrapper::CreateSurfaces(gfx::Size size,
                                  size_t num_surfaces,
                                  std::vector<VASurfaceID>* va_surfaces) {
  base::AutoLock auto_lock(va_lock_);
  DVLOG(2) << "Creating " << num_surfaces << " surfaces";

  DCHECK(va_surfaces->empty());
  DCHECK(va_surface_ids_.empty());
  va_surface_ids_.resize(num_surfaces);

  // Allocate surfaces in driver.
  VAStatus va_res = vaCreateSurfaces(va_display_,
                                     VA_RT_FORMAT_YUV420,
                                     size.width(), size.height(),
                                     &va_surface_ids_[0],
                                     va_surface_ids_.size(),
                                     NULL, 0);

  VA_LOG_ON_ERROR(va_res, "vaCreateSurfaces failed");
  if (va_res != VA_STATUS_SUCCESS) {
    va_surface_ids_.clear();
    return false;
  }

  // And create a context associated with them.
  va_res = vaCreateContext(va_display_, va_config_id_,
                           size.width(), size.height(), VA_PROGRESSIVE,
                           &va_surface_ids_[0], va_surface_ids_.size(),
                           &va_context_id_);

  VA_LOG_ON_ERROR(va_res, "vaCreateContext failed");
  if (va_res != VA_STATUS_SUCCESS) {
    DestroySurfaces();
    return false;
  }

  *va_surfaces = va_surface_ids_;
  return true;
}

void VaapiWrapper::DestroySurfaces() {
  base::AutoLock auto_lock(va_lock_);
  DVLOG(2) << "Destroying " << va_surface_ids_.size()  << " surfaces";

  if (va_context_id_ != VA_INVALID_ID) {
    VAStatus va_res = vaDestroyContext(va_display_, va_context_id_);
    VA_LOG_ON_ERROR(va_res, "vaDestroyContext failed");
  }

  if (!va_surface_ids_.empty()) {
    VAStatus va_res = vaDestroySurfaces(va_display_, &va_surface_ids_[0],
                                        va_surface_ids_.size());
    VA_LOG_ON_ERROR(va_res, "vaDestroySurfaces failed");
  }

  va_surface_ids_.clear();
  va_context_id_ = VA_INVALID_ID;
}

bool VaapiWrapper::SubmitBuffer(VABufferType va_buffer_type,
                                size_t size,
                                void* buffer) {
  base::AutoLock auto_lock(va_lock_);

  VABufferID buffer_id;
  VAStatus va_res = vaCreateBuffer(va_display_, va_context_id_,
                                   va_buffer_type, size,
                                   1, buffer, &buffer_id);
  VA_SUCCESS_OR_RETURN(va_res, "Failed to create a VA buffer", false);

  switch (va_buffer_type) {
    case VASliceParameterBufferType:
    case VASliceDataBufferType:
      pending_slice_bufs_.push_back(buffer_id);
      break;

    default:
      pending_va_bufs_.push_back(buffer_id);
      break;
  }

  return true;
}

void VaapiWrapper::DestroyPendingBuffers() {
  base::AutoLock auto_lock(va_lock_);

  for (size_t i = 0; i < pending_va_bufs_.size(); ++i) {
    VAStatus va_res = vaDestroyBuffer(va_display_, pending_va_bufs_[i]);
    VA_LOG_ON_ERROR(va_res, "vaDestroyBuffer failed");
  }

  for (size_t i = 0; i < pending_slice_bufs_.size(); ++i) {
    VAStatus va_res = vaDestroyBuffer(va_display_, pending_slice_bufs_[i]);
    VA_LOG_ON_ERROR(va_res, "vaDestroyBuffer failed");
  }

  pending_va_bufs_.clear();
  pending_slice_bufs_.clear();
}

bool VaapiWrapper::SubmitDecode(VASurfaceID va_surface_id) {
  base::AutoLock auto_lock(va_lock_);

  DVLOG(4) << "Pending VA bufs to commit: " << pending_va_bufs_.size();
  DVLOG(4) << "Pending slice bufs to commit: " << pending_slice_bufs_.size();
  DVLOG(4) << "Decoding into VA surface " << va_surface_id;

  // Get ready to decode into surface.
  VAStatus va_res = vaBeginPicture(va_display_, va_context_id_,
                                   va_surface_id);
  VA_SUCCESS_OR_RETURN(va_res, "vaBeginPicture failed", false);

  // Commit parameter and slice buffers.
  va_res = vaRenderPicture(va_display_, va_context_id_,
                           &pending_va_bufs_[0], pending_va_bufs_.size());
  VA_SUCCESS_OR_RETURN(va_res, "vaRenderPicture for va_bufs failed", false);

  va_res = vaRenderPicture(va_display_, va_context_id_,
                           &pending_slice_bufs_[0],
                           pending_slice_bufs_.size());
  VA_SUCCESS_OR_RETURN(va_res, "vaRenderPicture for slices failed", false);

  // Instruct HW decoder to start processing committed buffers (decode this
  // picture). This does not block until the end of decode.
  va_res = vaEndPicture(va_display_, va_context_id_);
  VA_SUCCESS_OR_RETURN(va_res, "vaEndPicture failed", false);

  return true;
}

bool VaapiWrapper::DecodeAndDestroyPendingBuffers(VASurfaceID va_surface_id) {
  bool result = SubmitDecode(va_surface_id);
  DestroyPendingBuffers();
  return result;
}

bool VaapiWrapper::CreateRGBImage(gfx::Size size, VAImage* image) {
  base::AutoLock auto_lock(va_lock_);
  VAStatus va_res;
  VAImageFormat format;
  format.fourcc = VA_FOURCC_BGRX;
  format.byte_order = VA_LSB_FIRST;
  format.bits_per_pixel = 32;
  format.depth = 24;
  format.red_mask = 0x0000ff;
  format.green_mask = 0x00ff00;
  format.blue_mask = 0xff0000;
  format.alpha_mask = 0;

  va_res = vaCreateImage(va_display_,
                         &format,
                         size.width(),
                         size.height(),
                         image);
  VA_SUCCESS_OR_RETURN(va_res, "Failed to create image", false);
  return true;
}

void VaapiWrapper::DestroyImage(VAImage* image) {
  base::AutoLock auto_lock(va_lock_);
  vaDestroyImage(va_display_, image->image_id);
}

bool VaapiWrapper::MapImage(VAImage* image, void** buffer) {
  base::AutoLock auto_lock(va_lock_);

  VAStatus va_res = vaMapBuffer(va_display_, image->buf, buffer);
  VA_SUCCESS_OR_RETURN(va_res, "Failed to map image", false);
  return true;
}

void VaapiWrapper::UnmapImage(VAImage* image) {
  base::AutoLock auto_lock(va_lock_);
  vaUnmapBuffer(va_display_, image->buf);
}

bool VaapiWrapper::PutSurfaceIntoImage(VASurfaceID va_surface_id,
                                       VAImage* image) {
  base::AutoLock auto_lock(va_lock_);
  VAStatus va_res = vaSyncSurface(va_display_, va_surface_id);
  VA_SUCCESS_OR_RETURN(va_res, "Failed syncing surface", false);

  va_res = vaGetImage(va_display_,
                      va_surface_id,
                      0,
                      0,
                      image->width,
                      image->height,
                      image->image_id);
  VA_SUCCESS_OR_RETURN(va_res, "Failed to put surface into image", false);
  return true;
}

bool VaapiWrapper::LockBuffer(VASurfaceID va_surface_id,
                              VABufferID buf_id,
                              VABufferInfo* buf_info) {
  DCHECK(buf_info);
  base::AutoLock auto_lock(va_lock_);

  buf_info->mem_type = VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM;
  VAStatus va_res = vaLockBuffer(va_display_, buf_id, buf_info);
  VA_SUCCESS_OR_RETURN(va_res, "Failed to lock vabuffer", false);

  return true;
}

bool VaapiWrapper::UnlockBuffer(VASurfaceID va_surface_id,
                                VABufferID buf_id,
                                VABufferInfo* buf_info) {
  DCHECK(buf_info);
  base::AutoLock auto_lock(va_lock_);
  VAStatus va_res = vaUnlockBuffer(va_display_, buf_id, buf_info);
  VA_SUCCESS_OR_RETURN(va_res, "Failed to unlock vabuffer", false);

  return true;
}


bool VaapiWrapper::GetVaImageForTesting(VASurfaceID va_surface_id,
                                        VAImage* image,
                                        void** mem) {
  base::AutoLock auto_lock(va_lock_);

  VAStatus va_res = vaSyncSurface(va_display_, va_surface_id);
  VA_SUCCESS_OR_RETURN(va_res, "Failed syncing surface", false);

  // Derive a VAImage from the VASurface
  va_res = vaDeriveImage(va_display_, va_surface_id, image);
  VA_LOG_ON_ERROR(va_res, "vaDeriveImage failed");
  if (va_res != VA_STATUS_SUCCESS)
    return false;

  // Map the VAImage into memory
  va_res = vaMapBuffer(va_display_, image->buf, mem);
  VA_LOG_ON_ERROR(va_res, "vaMapBuffer failed");
  if (va_res == VA_STATUS_SUCCESS)
    return true;

  vaDestroyImage(va_display_, image->image_id);
  return false;
}

void VaapiWrapper::ReturnVaImageForTesting(VAImage* image) {
  base::AutoLock auto_lock(va_lock_);

  vaUnmapBuffer(va_display_, image->buf);
  vaDestroyImage(va_display_, image->image_id);
}

// static
bool VaapiWrapper::PostSandboxInitialization() {
  StubPathMap paths;
  paths[kModuleVa_wayland].push_back(kVaLib);
  bool ret = InitializeStubs(paths);
  if (ret == false)
    LOG(WARNING) << "Could not open " << kVaLib;
  return ret;
}

bool VaapiWrapper::SupportsVaLockBufferApis() {
  void* handle = dlopen(kVaLib, RTLD_LAZY);
  if (!handle) {
    LOG(ERROR) << "Could not open " << kVaLib;
    return false;
  }
  return dlsym(handle, kVaLockBufferSymbol) &&
         dlsym(handle, kVaUnlockBufferSymbol);
}

}  // namespace media
