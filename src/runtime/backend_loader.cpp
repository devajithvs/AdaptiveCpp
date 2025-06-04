/*
 * This file is part of AdaptiveCpp, an implementation of SYCL and C++ standard
 * parallelism for CPUs and GPUs.
 *
 * Copyright The AdaptiveCpp Contributors
 *
 * AdaptiveCpp is released under the BSD 2-Clause "Simplified" License.
 * See file LICENSE in the project root for full license details.
 */
// SPDX-License-Identifier: BSD-2-Clause
#include "hipSYCL/runtime/backend_loader.hpp"
#include "hipSYCL/common/config.hpp"
#include "hipSYCL/common/debug.hpp"
#include "hipSYCL/common/dylib_loader.hpp"
#include "hipSYCL/common/filesystem.hpp"
#include "hipSYCL/runtime/application.hpp"
#include "hipSYCL/runtime/device_id.hpp"

// We always need a CPU backend
#include "hipSYCL/runtime/omp/omp_backend.hpp"
#ifdef WITH_CUDA_BACKEND
#include "hipSYCL/runtime/cuda/cuda_backend.hpp"
#endif
#ifdef WITH_ROCM_BACKEND
#include "hipSYCL/runtime/hip/hip_backend.hpp"
#endif
#ifdef WITH_LEVEL_ZERO_BACKEND
#include "hipSYCL/runtime/ze/ze_backend.hpp"
#endif
#ifdef WITH_OPENCL_BACKEND
#include "hipSYCL/runtime/ocl/ocl_backend.hpp"
#endif

#include <cassert>

#ifndef _WIN32
#include <dlfcn.h>
#else
#include <windows.h>
#endif

#include HIPSYCL_CXX_FILESYSTEM_HEADER
namespace fs = HIPSYCL_CXX_FILESYSTEM_NAMESPACE;

namespace {

using namespace hipsycl::common;

bool is_plugin_active(const std::string& name)
{
  auto backends_active = hipsycl::rt::application::get_settings().get<hipsycl::rt::setting::visibility_mask>();
  if(backends_active.empty())
    return true;
  if(name == "omp") // we always need a cpu backend
    return true;

  hipsycl::rt::backend_id id;
  if(name == "cuda") {
    id = hipsycl::rt::backend_id::cuda;
  } else if(name == "hip") {
    id = hipsycl::rt::backend_id::hip;
  } else if(name == "ze") {
    id = hipsycl::rt::backend_id::level_zero;
  } else if(name == "ocl") {
    id = hipsycl::rt::backend_id::ocl;
  }
  return backends_active.find(id) != backends_active.cend();
}

}

namespace hipsycl {
namespace rt {

void backend_loader::query_backends() {
  if(is_plugin_active("omp")) {
    _handles.push_back({"omp", nullptr});
    HIPSYCL_DEBUG_INFO << "backend_loader: Successfully opened plugin: " << "omp\n";
  }

#ifdef WITH_CUDA_BACKEND
  if(is_plugin_active("cuda")) {
    _handles.push_back({"cuda", nullptr});
    HIPSYCL_DEBUG_INFO << "backend_loader: Successfully opened plugin: " << "cuda\n";
  }
#endif

#ifdef WITH_ROCM_BACKEND
  if(is_plugin_active("hip")) {
    _handles.push_back({"hip", nullptr});
    HIPSYCL_DEBUG_INFO << "backend_loader: Successfully opened plugin: " << "hip\n";
  }
#endif

#ifdef WITH_OPENCL_BACKEND
  if(is_plugin_active("ocl")) {
    _handles.push_back({"ocl", nullptr});
    HIPSYCL_DEBUG_INFO << "backend_loader: Successfully opened plugin: " << "ocl\n";
  }
#endif

#ifdef WITH_LEVEL_ZERO_BACKEND
  if(is_plugin_active("ze")) {
    _handles.push_back({"ze", nullptr});
    HIPSYCL_DEBUG_INFO << "backend_loader: Successfully opened plugin: " << "ze\n";
  }
#endif

}

backend_loader::~backend_loader() { }

std::size_t backend_loader::get_num_backends() const { return _handles.size(); }

std::string backend_loader::get_backend_name(std::size_t index) const {
  assert(index < _handles.size());
  return _handles[index].first;
}

bool backend_loader::has_backend(const std::string &name) const {
  for (const auto &h : _handles) {
    if (h.first == name)
      return true;
  }

  return false;
}

backend *backend_loader::create(std::size_t index) const {
  assert(index < _handles.size());
  auto backend_name = _handles[index].first;
  if (backend_name == "omp")
    return new hipsycl::rt::omp_backend();

#ifdef WITH_CUDA_BACKEND
  if (backend_name == "cuda")
    return new hipsycl::rt::cuda_backend();
#endif

#ifdef WITH_ROCM_BACKEND
  if (backend_name == "hip")
    return new hipsycl::rt::hip_backend();
#endif

#ifdef WITH_LEVEL_ZERO_BACKEND
  if (backend_name == "ze")
    return new hipsycl::rt::ze_backend();
#endif

#ifdef WITH_OPENCL_BACKEND
  if (backend_name == "ocl")
    return new hipsycl::rt::ocl_backend();
#endif

  return nullptr;
}

backend *backend_loader::create(const std::string &name) const {
  
  for (std::size_t i = 0; i < _handles.size(); ++i) {
    if (_handles[i].first == name)
      return create(i);
  }

  return nullptr;
}

} // namespace rt
} // namespace hipsycl
