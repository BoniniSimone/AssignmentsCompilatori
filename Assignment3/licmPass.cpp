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

        for (Loop *L : LI) {
            bool Changed = true;
            while(Changed) {
                Changed = false; // Resetto il flag per ogni iterazione del loop
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

                SmallVector<BasicBlock *, 4> ExitBlocks;
                L->getExitBlocks(ExitBlocks);

                SmallVector<Instruction *, 8> Candidates;

                // Scorriamo tutte le istruzioni nel loop
                for (BasicBlock *BB : L->blocks()) {
                    for (Instruction &I : *BB) {
                        // Salta istruzioni che non possono essere spostate
                        if (isa<PHINode>(&I) || I.isTerminator() || I.getType()->isVoidTy())  // Non consideriamo PHI nodes, terminatori (es. br) o istruzioni che non restituiscono un valore (es. call)
                            continue;

                        if (I.mayHaveSideEffects() || I.mayReadFromMemory())
                            continue;


                        // 1. L'instruzione deve essere loop invariant (quindi i suoi operandi o sono costati o sono definiti fuori dal loop)
                        /*
                        bool HasVariantOp = false;
                        for (Value *Op : I.operands())
                        if (!L->isLoopInvariant(Op)) {     // questa call è corretta per gli *operandi*
                            HasVariantOp = true;
                            break;
                        }
                        if (HasVariantOp)
                        continue;*/
                        
                        bool VariantOperand = false;
                        for (Value *Operando : I.operands()) {
                            if (Instruction *OpInst = dyn_cast<Instruction>(Operando)) {  //se l'operando non è un'istruzione, allora è già loop invariant (es le costanti x=6)
                                if (L->contains(OpInst)) { // Se l'operando è un'istruzione nel loop, non è loop invariant
                                    
                                        VariantOperand = true;
                                        break;
                                    
                                }
                            }
                        }
                        if (VariantOperand) continue;



                        //2. Controllare che 'I' domini tutti i suoi usi nel loop

                        bool DominatesAllUses = true;
                        for (User *U : I.users()) {
                            if (Instruction *UI = dyn_cast<Instruction>(U)) {
                                if (L->contains(UI) && !DT.dominates(I.getParent(), UI->getParent())) { //DT.dominates(A,B) verifica se il blocco A domina il blocco B (SICOOME UN BLOCCO DOMINA SEMPRE SE STESSO SE RESITUTISCONO LO STESSO BB, LA CONDIZIONE è true!)
                                    DominatesAllUses = false;
                                    errs() << "L'istruzione " << I << " non domina l'uso " << *UI << "\n";
                                    break;
                                }
                            }
                        }
                        if (!DominatesAllUses) continue;

                        
                        //3. Controllare che l'istruzione domini tutti i blocchi di uscita del loop o che sia morta al di fuori del loop

                        bool DomAllExits = true;
                        for (BasicBlock *EB : ExitBlocks) {
                            if (!DT.dominates(I.getParent(), EB)) {
                            DomAllExits = false;
                            errs() << "L'istruzione " << I << " non domina il blocco di uscita " << *EB << "\n";
                            break;
                            }
                        }
                        bool DeadOutside = true;
                        for (User *U : I.users()) {
                            if (auto *UI = dyn_cast<Instruction>(U))
                            if (!L->contains(UI)) {
                                DeadOutside = false;
                                errs() << "L'istruzione " << I << " è usata fuori dal loop in " << *UI << "\n";
                                break;
                            }
                        }
                        if (!DomAllExits && !DeadOutside) continue;

                        
                        Candidates.push_back(&I); // Aggiungi l'istruzione alla lista dei candidati
                    }
                }

                // Sposta le istruzioni nel preheader
                for (Instruction *I : Candidates) {
                    I->moveBefore(Preheader->getTerminator());
                    errs() << "Spostata nel preheader: " << *I << "\n";
                    Changed = true; // Abbiamo fatto un cambiamento, quindi dobbiamo ripetere il processo

                }
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