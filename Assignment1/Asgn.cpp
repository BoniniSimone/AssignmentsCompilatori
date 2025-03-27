#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::PatternMatch;

namespace {

//-----------------------------------------------------------------------------
// Pass 1: AlgebraicIdentity
// Riconosce: x + 0 = 0 + x e lo sostituisce con x
// Riconosce: x * 1 = 1 * x e lo sostituisce con x
//-----------------------------------------------------------------------------
struct AlgebraicId : PassInfoMixin<AlgebraicId> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    bool Changed = false;
    for (auto &BB : F) {
      // Utilizziamo un iteratore "safe" per poter eliminare le istruzioni
      for (auto Inst = BB.begin(); Inst != BB.end(); ) {
        Instruction *I = &*Inst++;
        //Cosa fa per ogni istruzione?
      }
    }
    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }
};

//-----------------------------------------------------------------------------
// Pass 2: StrenghReduction
// Riconosce: 15 * x e lo sostituisce con (x << 4) - x
//-----------------------------------------------------------------------------
struct StrengthRed : PassInfoMixin<StrengthRed> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    bool Changed = false;
    for (auto &BB : F) {
      for (auto Inst = BB.begin(); Inst != BB.end(); ) {
        Instruction *I = &*Inst++;
        //Cosa fa per ogni istruzione?
      }
    }
    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }
};

//-----------------------------------------------------------------------------
// Pass 3: Multi-Instruction Optimization
// Riconosce: a = b + 1, c = a - 1  e sostituisce c con b
//-----------------------------------------------------------------------------
struct MultiInsOpt : PassInfoMixin<MultiInsOpt> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    bool Changed = false;
    for (auto &BB : F) {
      for (auto Inst = BB.begin(); Inst != BB.end(); ) {
        Instruction *I = &*Inst++;
        //Cosa fa per ogni istruzione?
      }
    }
    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }
};

} // end anonymous namespace

//-----------------------------------------------------------------------------
// Registrazione dei pass per il New Pass Manager
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getAsgnPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, "Asgn", LLVM_VERSION_STRING,
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
          [](StringRef Name, FunctionPassManager &FPM,
             ArrayRef<PassBuilder::PipelineElement>) {
            if (Name == "algebraic-id") {
              FPM.addPass(AlgebraicId());
              return true;
            }
            if (Name == "strength-red") {
              FPM.addPass(StrengthRed());
              return true;
            }
            if (Name == "multi-ins-opt") {
              FPM.addPass(MultiInsOpt());
              return true;
            }
            return false;
          });
    }
  };
}

// Interfaccia principale per i pass plugin.
// Questo garantisce che 'opt' riconosca le pass quando usate da linea di comando,
// ad es. tramite "-passes=zero-add" oppure "-passes=mul15" o "-passes=pattern".
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getAsgnPluginInfo();
}
