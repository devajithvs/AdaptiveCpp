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
#include "hipSYCL/compiler/llvm-to-backend/host/HostKnownWgSizePass.hpp"
#include "hipSYCL/compiler/cbs/IRUtils.hpp"
#include "hipSYCL/compiler/cbs/SplitterAnnotationAnalysis.hpp"
#include <array>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Type.h>

namespace hipsycl {
namespace compiler {

namespace {

constexpr llvm::StringRef PassPrefix = "[SSCP][HostWgSizeOpt] ";

void replaceWgSizeGlobalsWithConstants(llvm::Function &F, const std::array<int, 3> &KnownWGSize) {
  auto DL = F.getParent()->getDataLayout();
  auto SizeT = DL.getLargestLegalIntType(F.getContext());

  for (auto i = 0ul; i < 3ul; ++i) {
    if (KnownWGSize.at(i) != 0)
      utils::replaceUsesOfGVWith(F, cbs::LocalSizeGlobalNames.at(i),
                                 llvm::ConstantInt::get(SizeT, KnownWGSize.at(i)), PassPrefix);
  }
}

} // namespace

llvm::PreservedAnalyses HostKnownWgSizePass::run(llvm::Function &F,
                                                 llvm::FunctionAnalysisManager &AM) {

  auto &MAM = AM.getResult<llvm::ModuleAnalysisManagerFunctionProxy>(F);
  auto *SAA = MAM.getCachedResult<SplitterAnnotationAnalysis>(*F.getParent());
  if (!SAA || !SAA->isKernelFunc(&F))
    return llvm::PreservedAnalyses::all();

  replaceWgSizeGlobalsWithConstants(F, KnownWgSize);

  HIPSYCL_DEBUG_INFO << PassPrefix << "Replaced work-group size GVs with Constants\n";

  return llvm::PreservedAnalyses::none();
}

} // namespace compiler
} // namespace hipsycl
