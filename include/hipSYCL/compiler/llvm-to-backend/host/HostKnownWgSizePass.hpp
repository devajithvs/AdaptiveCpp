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
#ifndef HIPSYCL_HOST_KNOWN_WG_SIZE_HPP
#define HIPSYCL_HOST_KNOWN_WG_SIZE_HPP

#include <llvm/IR/PassManager.h>

namespace hipsycl {
namespace compiler {

/**
 * SubCfgFormationPass internally uses the work-group size global variables.
 * For example, we use them for loop trip counts.
 * Since we know their value at run-time, we just replace all uses of the global variables with
 * their respective constant value.
 */
class HostKnownWgSizePass : public llvm::PassInfoMixin<HostKnownWgSizePass> {
  std::array<int, 3> KnownWgSize;

public:
  explicit HostKnownWgSizePass(int KnownGroupSizeX, int KnownGroupSizeY, int KnownGroupSizeZ)
      : KnownWgSize{KnownGroupSizeX, KnownGroupSizeY, KnownGroupSizeZ} {}

  llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &AM);
};

} // namespace compiler
} // namespace hipsycl

#endif
