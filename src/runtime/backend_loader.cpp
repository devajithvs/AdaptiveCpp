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
#include "hipSYCL/runtime/application.hpp"
#include "hipSYCL/runtime/dylib_loader.hpp"
#include "hipSYCL/common/debug.hpp"
#include "hipSYCL/common/config.hpp"
#include "hipSYCL/runtime/device_id.hpp"
#include "hipSYCL/runtime/cuda/cuda_backend.hpp"
#include "hipSYCL/runtime/omp/omp_backend.hpp"

#include <cassert>

#ifndef _WIN32
#include <dlfcn.h>
#else
#include <windows.h> 
#endif

#include HIPSYCL_CXX_FILESYSTEM_HEADER
namespace fs = HIPSYCL_CXX_FILESYSTEM_NAMESPACE;

namespace {

using namespace hipsycl::rt::detail;

hipsycl::rt::backend *cuda_backend_factory() {
  return new hipsycl::rt::cuda_backend();
}

hipsycl::rt::backend *omp_backend_factory() {
  return new hipsycl::rt::omp_backend();
}

hipsycl::rt::backend *create_backend(void *plugin_handle) {
  assert(plugin_handle);

  if(void *symbol = get_symbol_from_library(plugin_handle, "hipsycl_backend_plugin_create", "backend_loader"))
  {
    auto create_backend_func =
        reinterpret_cast<decltype(&hipsycl_backend_plugin_create)>(symbol);

    return create_backend_func();
  }
  return nullptr;
}

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
    _handles.push_back({"omp", reinterpret_cast<void*>(&omp_backend_factory)});
    HIPSYCL_DEBUG_INFO << "backend_loader: Successfully opened plugin: " << "omp\n";
  }

  if(is_plugin_active("cuda")) {
    _handles.push_back({"cuda", reinterpret_cast<void*>(&cuda_backend_factory)});
    HIPSYCL_DEBUG_INFO << "backend_loader: Successfully opened plugin: " << "cuda\n";
  }

}

backend_loader::~backend_loader() {
  for (auto &handle : _handles) {
    assert(handle.second);
  }
}

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
  if (backend_name == "cuda")
    return new hipsycl::rt::cuda_backend();
  if (backend_name == "omp")
    return new hipsycl::rt::omp_backend();
  return nullptr;
}

backend *backend_loader::create(const std::string &name) const {
  
  for (std::size_t i = 0; i < _handles.size(); ++i) {
    if (_handles[i].first == name)
      return create(i);
  }

  return nullptr;
}

}
} // namespace hipsycl
