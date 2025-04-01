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
// Pass 1: AlgebraicIdentity

// La struttura AlgebraicId è un passaggio di ottimizzazione che implementa l'interfaccia PassInfoMixin.
struct AlgebraicId : PassInfoMixin<AlgebraicId> {
  
  // Il metodo run è chiamato quando il passaggio viene eseguito. Prende come parametri
  // una funzione F (la funzione da ottimizzare) e un FunctionAnalysisManager (gestisce le analisi della funzione).
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    bool Changed = false;  // Variabile che tiene traccia se sono state fatte modifiche.

    // Itera attraverso tutti i blocchi base della funzione.
    for (auto &BB : F) {
      
      // Iteratore per scorrere le istruzioni nel blocco base, per eliminare possibilmente istruzioni.
      for (auto Inst = BB.begin(); Inst != BB.end(); ) {
        Instruction *I = &*Inst++;  // Salva l'istruzione corrente e avanza l'iteratore.
        
        // Considera solo operazioni binarie (Add, Mul, ecc.)
        if (auto *BinOp = dyn_cast<BinaryOperator>(I)) {
          
          // Caso: addizione (x + 0 oppure 0 + x)
          if (BinOp->getOpcode() == Instruction::Add) {
            Value *Op;  // Variabile per contenere l'operando non nullo (x o 0).
            x = x+0
            // Controlla se l'operazione è una somma di un valore e zero (x + 0).
            if (match(BinOp, m_Add(m_Value(Op), m_Zero()))) {
              BinOp->replaceAllUsesWith(Op);  // Sostituisce tutte le occorrenze dell'istruzione con il valore Op (x).
              BinOp->eraseFromParent();  // Elimina l'istruzione BinOp.
              Changed = true; 
              continue;  
            }
            
            // Controlla se l'operazione è una somma di zero e un valore (0 + x).
            if (match(BinOp, m_Add(m_Zero(), m_Value(Op)))) {
              BinOp->replaceAllUsesWith(Op);  
              BinOp->eraseFromParent();  
              Changed = true;  
              continue;  
            }
          }
          
          // Caso: moltiplicazione per uno (x * 1 oppure 1 * x)
          if (BinOp->getOpcode() == Instruction::Mul) {
            Value *Op;
            if (match(BinOp, m_Mul(m_Value(Op), m_One()))) {
              BinOp->replaceAllUsesWith(Op);
              BinOp->eraseFromParent();
              Changed = true;
              continue;
            }
            if (match(BinOp, m_Mul(m_One(), m_Value(Op)))) {
              BinOp->replaceAllUsesWith(Op);
              BinOp->eraseFromParent();
              Changed = true;
              continue;
            }
          }
        }
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
