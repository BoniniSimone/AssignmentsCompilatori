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

  struct LoopFusion : PassInfoMixin<LoopFusion> {

    PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);

    //Raccogliamo tutti i loop in ordine sequenziale
    SmallVector<Loop*,8> AllLoops;
    for (BasicBlock &BB : F) {
      if (Loop *L = LI.getLoopFor(&BB)) { //getLoopFor restituisce per ogni BasicBlock il loop a cui appartiene (ma header, body, latch e exit appattengono tutti allo stesso loop quindi...)
        if (L->getHeader() == &BB) { //Aggiungiamo solo una volta il loop quando trovimao il suo header
          if (find(AllLoops, L) == AllLoops.end()) //inolre controlliamo che non sia già presente. find() restituisce un iteratore che punta alla posizione dell'elemento trovato, se non lo trova restituisce end()
            AllLoops.push_back(L);
        }
      }
    }

    //Se ce ne sono almeno 2
    if (AllLoops.size() >= 2) {
      Loop *L0 = AllLoops[0];
      Loop *L1 = AllLoops[1];

      //Exit-blocks di L0
      SmallVector<BasicBlock*,4> Exits0;
      L0->getExitBlocks(Exits0);

      //Preheader di L1
      BasicBlock *Prehdr1 = L1->getLoopPreheader();

      /*
      // --- Stampa exit block di loop0 ---
      if (Exits0.empty()) {
        errs() << "Loop0 non ha exit-block!\n";
      } else {
        BasicBlock *Exit0 = Exits0[0];
        errs() << "Exit block di loop0:\n";
        Exit0->print(errs());
        errs() << "\n";
      }

      // --- Stampa preheader di loop1 ---
      if (!Prehdr1) {
        errs() << "Loop1 non ha preheader (non è in simplify-form)\n";
      } else {
        errs() << "Preheader di loop1:\n";
        Prehdr1->print(errs());
        errs() << "\n";
      }
      */

      
      bool direct = false;
      for (BasicBlock *E : Exits0) {
        if(E == Prehdr1) { //Se l'uscita di loop0 è il preheader di loop1
          direct = true;
          break;
        }
      }

      //Stampa risultato
      if (direct)
        errs() << "Loop0 → loop1 adiacenti (nessuna istruzione intermedia)\n";
      else
        errs() << "Tra loop0 e loop1 ci sono istruzioni intermedie\n";
    }

    return PreservedAnalyses::all();
}


    static bool isRequired() { return true; }
  };

  } // namespace

  extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {
      LLVM_PLUGIN_API_VERSION, "LoopFusion", LLVM_VERSION_STRING,
      [](PassBuilder &PB) {
        PB.registerPipelineParsingCallback(
          [](StringRef Name, FunctionPassManager &FPM,
            ArrayRef<PassBuilder::PipelineElement>) {
            if (Name == "fusion-pass") {
              FPM.addPass(LoopFusion());
              return true;
            }
            return false;
          });
      }
    };
  }