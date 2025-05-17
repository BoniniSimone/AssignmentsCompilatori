#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

struct LICMPass : PassInfoMixin<LICMPass> {

    PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
        LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
        DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);

        for (Loop *L : LI) { // Scorriamo tutti i loop
            if (!L->isLoopSimplifyForm()) {
                errs() << "Il loop non in forma normale\n";
                continue;
            }

            // Verifica che il loop abbia un preheader
            BasicBlock *Preheader = L->getLoopPreheader();
            if (!Preheader) {
                errs() << "Non è presente un preheader\n";
                continue;
            }

            SmallVector<Instruction *, 8> Candidates;

            // Scorriamo tutte le istruzioni nel loop
            for (BasicBlock *BB : L->blocks()) {
                for (Instruction &I : *BB) {
                    // Salta istruzioni che non possono essere spostate
                    if (isa<PHINode>(&I) || I.isTerminator() || I.getType()->isVoidTy())
                        continue;

                    if (I.mayHaveSideEffects() || I.mayReadFromMemory())
                        continue;

                    // 1. L'instruzione deve essere loop invariant (quindi i suoi operandi o sono costati o non sono variabili di loop)
                    bool VariantOperand = false;
                    for (Value *Op : I.operands()) {
                        if (Instruction *OpInst = dyn_cast<Instruction>(Op)) {
                            if (L->contains(OpInst)) {
                                VariantOperand = true;
                                break;
                            }
                        }
                    }
                    if (VariantOperand) continue;

                    // 2. Controllare che I domini tutti i suoi usi nel loop
                    bool DominatesAllUses = true;
                    for (User *U : I.users()) {
                        if (Instruction *UI = dyn_cast<Instruction>(U)) {
                            if (L->contains(UI) && !DT.dominates(I.getParent(), UI->getParent())) {
                                DominatesAllUses = false;
                                break;
                            }
                        }
                    }
                    if (!DominatesAllUses)
                        continue;

                    Candidates.push_back(&I); // Aggiungi l'istruzione alla lista dei candidati
                }
            }

            // Sposta le istruzioni nel preheader
            for (Instruction *I : Candidates) {
                I->moveBefore(Preheader->getTerminator());
                errs() << "Spostata nel preheader: " << *I << "\n";
            }
        }

        return PreservedAnalyses::none();
    }

  static bool isRequired() { return true; }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, "LICMPass", LLVM_VERSION_STRING,
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](StringRef Name, FunctionPassManager &FPM,
           ArrayRef<PassBuilder::PipelineElement>) {
          if (Name == "licm-pass") {
            FPM.addPass(LICMPass());
            return true;
          }
          return false;
        });
    }
  };
}