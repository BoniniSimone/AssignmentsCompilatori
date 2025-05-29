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


  //Funzione che raccoglie i loop
  static SmallVector<Loop*,8> collectLoopsInFunction(Function &F, LoopInfo &LI) {
    SmallVector<Loop*,8> All;
    for (BasicBlock &BB : F) {
      if (Loop *L = LI.getLoopFor(&BB)) { //getLoopFor restituisce per ogni BasicBlock il loop a cui appartiene (ma header, body, latch e exit appattengono tutti allo stesso loop quindi...)
        if (L->getHeader() == &BB) { //Aggiungiamo solo una volta il loop quando trovimao il suo header
          if (find(All, L) == All.end()) //inolre controlliamo che non sia già presente. find() restituisce un iteratore che punta alla posizione dell'elemento trovato, se non lo trova restituisce end()
            All.push_back(L);
        }
      }
    }
    return All;
  }

  //Funzione che controlla se i loop se hanno condizioni di guardia
  //Ritorna 1 se sono entrambi non guarded, ritona 2 se sono entrami guarded, ritorna 0 se uno è guarded e l'altro no.
  int guarded(Loop *L0, Loop *L1) {
    if (L0->isGuarded() || L1->isGuarded()) {
      if(L0->isGuarded() && L1->isGuarded()) {
        return 2; //Entrambi guarded
      }
      return 0; //Solo uno dei due è guarded
    }
    return 1; //Entrambi non guarded
  }

  //Funzione che controlla se ci sono istruzioni intermedie tra due loop
  bool intInstructions(BasicBlock *BB) {
    for (Instruction &I : *BB) {
      if (isa<PHINode>(&I)) continue; // Le PHI non sono istruzioni di lavoro
      if (&I == BB->getTerminator()) continue; // Il terminatore va bene
      if(isa<ICmpInst>(&I) || isa<FCmpInst>(&I)) continue; // Le istruzioni di confronto sono accettabili
      return false; // Se troviamo un'istruzione che non è PHI o terminatore
    }
    return true;
  }

  namespace {

  struct LoopFusion : PassInfoMixin<LoopFusion> {

    PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);

    //Raccogliamo tutti i loop in ordine sequenziale
    SmallVector<Loop*,8> AllLoops = collectLoopsInFunction(F, LI);

    //Se ce ne sono almeno 2
    if (AllLoops.size() >= 2) {
      Loop *L0 = AllLoops[0];
      Loop *L1 = AllLoops[1];

      //Exit-blocks di L0
      SmallVector<BasicBlock*,4> Exits0;
      L0->getExitBlocks(Exits0);

      //Preheader di L1
      BasicBlock *Prehdr1 = L1->getLoopPreheader();

      bool direct = false;
      bool cleanExit = false;
      bool cleanSuccessor = false;
      BasicBlock *succ = nullptr;
      BasicBlock *Exit0 = nullptr;

      switch (guarded(L0, L1)) {

        //---- Solo uno dei due è guarded ----
        case 0:
          errs() << "Loop0 e loop1 non si possono fondere, perchè uno è guarded e l'altro no\n";
          return PreservedAnalyses::all();

        //---- Entrambi i loop non sono guarded ----
        case 1:
          errs() << "Loop0 e loop1 non hanno condizioni di guardia, si può fondere\n";
          
          for (BasicBlock *E : Exits0) {
            if(E == Prehdr1) { //Se l'uscita di loop0 è il preheader di loop1
              direct = true;
              break;
            }
          }
          
          Exit0 = Exits0[0];
          cleanExit = intInstructions(Exit0); //Controlliamo se ci sono istruzioni intermedie tra loop0 e loop1

          //Stampa risultato
          if (direct && cleanExit)
            errs() << "Loop0 → loop1 adiacenti (nessuna istruzione intermedia)\n";
          else
            errs() << "Tra loop0 e loop1 ci sono istruzioni intermedie\n";

          break;

        //---- Entrambi i loop sono guarded ----
        case 2:
          errs() << "Loop0 e loop1 sono entrambi guarded\n";
          
          BasicBlock *Successor0 = L0->getLoopGuardBranch()->getSuccessor(0);
          BasicBlock *Successor1 = L0->getLoopGuardBranch()->getSuccessor(1);

          
          if(Successor0 == L1->getLoopGuardBranch()->getParent() || Successor1 == L1->getLoopGuardBranch()->getParent()) {
            direct = true;
            if(Successor0 == L1->getLoopGuardBranch()->getParent()) succ=Successor0;
            else succ=Successor1;  
          } 
          
          cleanSuccessor = intInstructions(succ);
        
          if (direct && cleanSuccessor) {
            errs() << "Loop0 → loop1 adiacenti (nessuna istruzione intermedia)\n";
          } else {
            errs() << "Tra loop0 e loop1 ci sono istruzioni intermedie\n";
          }
          
          return PreservedAnalyses::all();
      }

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