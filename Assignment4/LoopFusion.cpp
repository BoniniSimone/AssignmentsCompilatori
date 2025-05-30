#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/PostDominators.h"

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

//Funzione che controlla il Control Flow Equivalence tra due loop
bool controlFlowEq(Loop *L0, Loop *L1, DominatorTree &DT, PostDominatorTree &PDT) {
  BasicBlock *Header0 = L0->getHeader();
  BasicBlock *Header1 = L1->getHeader();

  if (DT.dominates(Header0, Header1) && PDT.dominates(Header1, Header0)) return true;
  
  return false;
}

//Funzione che controlla se i due loop guarded hanno la stessa condizione di guardia
bool guardConditionEq(Loop *L0, Loop *L1) {

  Value *Cond0 = L0->getLoopGuardBranch()->getCondition();
  Value *Cond1 = L1->getLoopGuardBranch()->getCondition();

  auto *C0 = dyn_cast<ICmpInst>(Cond0);
  auto *C1 = dyn_cast<ICmpInst>(Cond1);

  if (C0->getPredicate() == C1->getPredicate() && C0->getOperand(0) == C1->getOperand(0) && C0->getOperand(1) == C1->getOperand(1)) 
      return true;

  return false;
}

namespace {

struct LoopFusion : PassInfoMixin<LoopFusion> {

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
    PostDominatorTree &PDT = AM.getResult<PostDominatorTreeAnalysis>(F);
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
      bool controlFlow = false;
      bool sameGuardCond = false;
      BasicBlock *succ = nullptr;
      BasicBlock *Exit0 = nullptr;

      switch (guarded(L0, L1)) {

        //----- Solo uno dei due è guarded -----
        case 0:
          errs() << "Loop0 e loop1 non si possono fondere, perchè uno è guarded e l'altro no\n";
          return PreservedAnalyses::all();


        //----- Entrambi i loop NON GUARDED -----
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
          controlFlow=controlFlowEq(L0, L1, DT, PDT); //Controlliamo se i due loop hanno lo stesso control flow
          
          //Stampa risultato
          if (direct && cleanExit && controlFlow) 
            errs() << "Loop0 → loop1 adiacenti (nessuna istruzione intermedia e hanno stesso control flow)\n";
          else
            errs() << "Tra loop0 e loop1 ci sono istruzioni intermedie o non hanno lo stesso control flow\n";

          break;


        //----- Entrambi i loop sono GUARDED -----
        case 2:
          errs() << "Loop0 e loop1 sono entrambi guarded\n";
          
          BasicBlock *Successor0 = L0->getLoopGuardBranch()->getSuccessor(0);
          BasicBlock *Successor1 = L0->getLoopGuardBranch()->getSuccessor(1);

          
          //Se i loop sono guarded il successore non loop del guard branch di L0 deve essere l’entry block di L1.
          if(Successor0 == L1->getLoopGuardBranch()->getParent() || Successor1 == L1->getLoopGuardBranch()->getParent()) {
            direct = true;
            if(Successor0 == L1->getLoopGuardBranch()->getParent()) succ=Successor0;
            else succ=Successor1;  
          } 
          
          cleanSuccessor = intInstructions(succ);
          sameGuardCond=guardConditionEq(L0, L1);
        
          if (direct && cleanSuccessor && sameGuardCond) {
            errs() << "Loop0 → loop1 adiacenti (nessuna istruzione intermedia e hanno stessa condizione di guardia)\n";
          } else {
            errs() << "Tra loop0 e loop1 ci sono istruzioni intermedie o non hanno la stessa condizione di guardia\n";
          }
          
          return PreservedAnalyses::all();
      }
      
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