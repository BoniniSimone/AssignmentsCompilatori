#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/DependenceAnalysis.h"


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

//Funzione che controlla con la Scalar Evolution (SCEV) se i due loop hanno lo stesso numero di iterazioni
bool SCEVCheck(Loop *L0, Loop *L1, ScalarEvolution &SE) {
  const SCEV *BECount0 = SE.getBackedgeTakenCount(L0);  //const SCEV -> contiene l'espressione che indica quante volte verrà eseguito il loop
  const SCEV *BECount1 = SE.getBackedgeTakenCount(L1);  //meglio di getSmallConstantTakenCount che restituisce un intero se il numero di iterazioni è costante, altrimenti non torna nulla
  if (isa<SCEVCouldNotCompute>(BECount0) || isa<SCEVCouldNotCompute>(BECount1))
  {
    errs() << "SCEV non può calcolare il numero di iterazioni per uno dei loop\n";
    return false; // Se SCEV non può calcolare il numero di iterazioni, ritorniamo false
  }

  if (BECount0 == BECount1)  return true;
  return false;
}

//Funzione che controlla se ci sono dipendenze tra le istruzioni di loop1 con istruzioni di loop1
bool hasDependence(Loop *L0, Loop *L1, DependenceInfo &DI) {
  SmallVector<Instruction *, 8> LSInsts0; // Load/Store instructions in loop0
  SmallVector<Instruction *, 8> LSInsts1; // Load/Store instructions in loop1

  for (BasicBlock *BB : L0->getBlocks()) {
    for (Instruction &I : *BB) {
      if (isa<LoadInst>(&I) || isa<StoreInst>(&I)) {
        LSInsts0.push_back(&I);
      }
    }
  }

  for (BasicBlock *BB : L1->getBlocks()) {
    for (Instruction &I : *BB) {
      if (isa<LoadInst>(&I) || isa<StoreInst>(&I)) {
        LSInsts1.push_back(&I);
      }
    }
  }

  for (Instruction *I0 : LSInsts0) {
    for (Instruction *I1 : LSInsts1) {
      if (DI.depends(I0, I1, true)) {
        errs() << "Dipendenza trovata tra " << *I0 << " e " << *I1 << "\n";
        return true; 
      }
    }
  }
  return false; 
}

//Funzione che restituisce il blocco body di un loop
BasicBlock *getLoopBody(Loop *L) {
  BasicBlock *Header   = L->getHeader();
  BasicBlock *Latch = L->getLoopLatch();
  BasicBlock *Body  = nullptr;

  for (BasicBlock *Succ : successors(Header)) {
    if (!L->contains(Succ))
      continue; 

    if (Succ == Latch) //Se l'header ha come successore il latch, allora il body è l'header (caso dei loop guarded)
      Body = Header;
    else 
      Body = Succ;
    break;
  }

  assert(Body && "Body non trovato");
  return Body;
}

//Funzione che percmette di unire i body di più di due loop
void moreLoop(BasicBlock *L1Body, BasicBlock *L0Body, BasicBlock *L0Latch) {
  Instruction *T0 = L0Body->getTerminator(); 
  if (T0->getSuccessor(0) == L0Latch) { 
    T0->eraseFromParent();                   
    BranchInst::Create(L1Body, L0Body); 
  } else {
    moreLoop( L1Body, T0->getSuccessor(0), L0Latch); 
  }
}

//Funzione che fonde i loop NON Guarded
void fuseLoops(Loop *L0, Loop *L1, Function &F, LoopInfo &LI) {

  BasicBlock *L0Body = getLoopBody(L0);
  BasicBlock *L1Body = getLoopBody(L1);
 
  // 1) Sostiuiamo tutti gli IV (Induction Variables) di L1 con quelli di L0

  PHINode *PhiI0 = L0->getCanonicalInductionVariable();
  PHINode *PhiI1 = L1->getCanonicalInductionVariable();

  PhiI1->replaceAllUsesWith(PhiI0); //non cancella PhiI1 in sé, ma semplicemente reindirizza tutti gli “usi” (uses) verso PhiI0.
  PhiI1->eraseFromParent();  //Cnacella la phi di %i1

  BasicBlock *L1Pre   = L1->getLoopPreheader();
  BasicBlock *L1Hdr   = L1->getHeader();
  BasicBlock *L1Latch = L1->getLoopLatch();
  BasicBlock *L1Exit  = L1->getExitBlock();

  BasicBlock *L0Latch = L0->getLoopLatch(); 

  // 2) Branch da L0Body → L1Body e da L1Body → L0Latch

  
  moreLoop(L1Body, L0Body, L0Latch); 
  
  
  Instruction *T1 = L1Body->getTerminator();
  T1->eraseFromParent();                  
  BranchInst::Create(L0Latch, L1Body);     

  // 3) Branch da L0Header → L1Exit
  
  Instruction *THeader0 = L0->getHeader()->getTerminator();
  for (int i=0; i < THeader0->getNumSuccessors(); i++) {
    if (THeader0->getSuccessor(i) == L1Pre) { 
      THeader0->setSuccessor(i, L1Exit); 
      break;
    }
  }

  // 4) Cancelliamo Preheader, Header e Latch di L1
  
  L1Pre->eraseFromParent();  
  L1Hdr->eraseFromParent();  
  L1Latch->eraseFromParent(); 
  
}

void fuseLoopsGuarded(Loop *L0, Loop *L1, Function &F, LoopInfo &LI) {

  BasicBlock *L0Preheader = L0->getLoopPreheader(); 
  BasicBlock *L0Header    = L0->getHeader();        
  BasicBlock *L0Latch     = L0->getLoopLatch();     
  BasicBlock *L0Exit      = L0->getExitBlock();     

  BasicBlock *L1Preheader = L1->getLoopPreheader(); 
  BasicBlock *L1Header    = L1->getHeader();        
  BasicBlock *L1Latch     = L1->getLoopLatch();     
  BasicBlock *L1Exit      = L1->getExitBlock();     

  // 1) Trovare e sostituire le PHI di L1 con quelle di L0

  PHINode *PhiI0 = L0->getCanonicalInductionVariable();
  PHINode *PhiI1 = L1->getCanonicalInductionVariable();

  PhiI1->replaceAllUsesWith(PhiI0);
  PhiI1->eraseFromParent();


  // 2) Reindirizziamo L0Latch verso L1Exit

  Instruction *TLatch0 = L0Latch->getTerminator();
  for (int i = 0; i < TLatch0->getNumSuccessors(); ++i) {
    if (TLatch0->getSuccessor(i) == L0Exit) {
      TLatch0->setSuccessor(i, L1Exit);
      break;
    }
  }
  

  // 3) Branch da L0Header → L1Header e da L1Header → L0Latch

  moreLoop(L1Header, L0Header, L0Latch); 

  Instruction *T1 = L1Header->getTerminator();
  T1->eraseFromParent();
  BranchInst::Create(L0Latch, L1Header); 


  // 4) Cancelliamo L1Preheader 
  //Ridirezioniamo tutti i predecessori a L1Exit (cioè rindiriziamo Guard1)

  for (BasicBlock *Pred : predecessors(L1Preheader)) {
    Instruction *BI = dyn_cast<BranchInst>(Pred->getTerminator());
    for (int i = 0; i < BI->getNumSuccessors(); ++i) {
      if (BI->getSuccessor(i) == L1Preheader) {
        BI->setSuccessor(i, L1Exit);
      }
    }
  }
  
  L1Preheader->eraseFromParent();
  

  // 5) Cancelliamo L1Latch (block 27):
  //Aveva come predecessore L1Header, che ora punta a L0Latch (vedere punto 4), quindi non ha più predecessori e possiamo eliminarlo senza agire sui predecessori.
  
  L1Latch->eraseFromParent();

  // 6) Cancelliamo L0Exit (block 20):
  // L0Exit aveva come predecessore L0Latch, che ora punta a L1Exit (vedere punto 3), quindi non ha più predecessorie possimao eliminarlo senza agire sui predecessori.
  
  L0Exit->eraseFromParent();
  

}

namespace {

struct LoopFusion : PassInfoMixin<LoopFusion> {

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    bool anyFuision = false;

    while(true){

      bool fusedThisRound = false;

      DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
      PostDominatorTree &PDT = AM.getResult<PostDominatorTreeAnalysis>(F);
      LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
      ScalarEvolution &SE = AM.getResult<ScalarEvolutionAnalysis>(F);
      DependenceInfo&DI = AM.getResult<DependenceAnalysis>(F);

      //Raccogliamo tutti i loop in ordine sequenziale
      SmallVector<Loop*,8> AllLoops = collectLoopsInFunction(F, LI);

      //Se ce ne sono almeno 2
      if (AllLoops.size() >= 2) {
        for (int i = 0; i+1 < AllLoops.size(); ++i) {
          Loop *L0 = AllLoops[i];
          Loop *L1 = AllLoops[i+1];
        
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
              break;


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
              if (direct && cleanExit && controlFlow){
                errs() << "Loop0 → loop1 adiacenti (nessuna istruzione intermedia e hanno stesso control flow)\n";
                if(SCEVCheck(L0, L1, SE)) {
                  errs() << "I loop hanno lo stesso numero di iterazioni\n";
                  if(hasDependence(L0, L1, DI)) {
                    errs() << "I loop hanno dipendenze tra le loro istruzioni\n";
                  } 
                  else {
                    errs() << "I loop non hanno dipendenze tra le loro istruzioni\n";
                    fuseLoops(L0, L1, F, LI); //Fusione dei loop
                    errs() << "Fusione dei loop completata con successo\n\n";
                    fusedThisRound = true;
                    anyFuision = true;
                    i++; //Incrementiamo i per saltare il loop successivo, che è stato fuso con questo
                  }
                } 
                else {
                  errs() << "I loop non hanno lo stesso numero di iterazioni\n";
                }
              }
              
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
                if(SCEVCheck(L0, L1, SE)) {
                  errs() << "I loop hanno lo stesso numero di iterazioni\n";
                  if(hasDependence(L0, L1, DI)) {
                    errs() << "I loop hanno dipendenze tra le loro istruzioni\n";
                  } else {
                    errs() << "I loop non hanno dipendenze tra le loro istruzioni\n";
                    fuseLoopsGuarded(L0, L1, F, LI); //Fusione dei loop
                    errs() << "Fusione dei loop completata con successo\n\n";
                    fusedThisRound = true;
                    anyFuision = true;
                    i++; //Incrementiamo i per saltare il loop successivo, che è stato fuso con questo
                  }
                } else {
                  errs() << "I loop non hanno lo stesso numero di iterazioni\n";
                }
              } else {
                errs() << "Tra loop0 e loop1 ci sono istruzioni intermedie o non hanno la stessa condizione di guardia\n";
              }
              
              break;
          }
          
        }
      }
    if(!fusedThisRound)
      break;
  }
    if(anyFuision) {
      return PreservedAnalyses::none();
    } else {
      return PreservedAnalyses::all();
    }
    
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