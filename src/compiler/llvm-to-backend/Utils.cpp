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

#include "hipSYCL/compiler/llvm-to-backend/Utils.hpp"
#include "hipSYCL/common/config.hpp"

namespace hipsycl {
namespace compiler {

std::string getClangPath() {
  static std::string path;
  if(!path.empty())
    return path;
  else
    path = ACPP_CLANG_PATH;
  
  auto backup_path = path;
  auto pos = path.find("$ACPP_PATH");
  while (pos != std::string::npos) {
    const auto install_dir = common::filesystem::get_install_directory();
    path.replace(pos, std::string_view("$ACPP_PATH").size(), install_dir);
    pos = path.find("$ACPP_PATH");
  }

  if (!common::filesystem::exists(path)) {
    path = backup_path;
    pos = path.find("$ACPP_PATH");
    while (pos != std::string::npos) {
      const auto clang_build_dir = common::filesystem::get_install_directory() + "/interpreter/llvm-project/llvm";
      path.replace(pos, std::string_view("$ACPP_PATH").size(), clang_build_dir);
      pos = path.find("$ACPP_PATH");
    }
  }
  return path;
}

std::string getBuiltinBitcodeFile(std::string BuiltinBitcodeFileName) {
  std::string BuiltinBitcodeFile = common::filesystem::join_path(common::filesystem::get_install_directory(),
  {"lib", "hipSYCL", "bitcode", BuiltinBitcodeFileName});

  if (!common::filesystem::exists(BuiltinBitcodeFile))
    BuiltinBitcodeFile =
    common::filesystem::join_path(ADAPTIVE_CPP_BINARY_DIR,
      {"lib", "hipSYCL", "bitcode", BuiltinBitcodeFileName});

  return BuiltinBitcodeFile;
}

} // namespace compiler
} // namespace hipsycl