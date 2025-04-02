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
struct MultiInsOpt : public PassInfoMixin<MultiInsOpt> {
  /// Funzione helper per ottenere la forma canonica (base, offset)
  std::pair<Value*, int64_t> getCanonicalForm(Value *V) {
    if (auto *BO = dyn_cast<BinaryOperator>(V)) {
      // Per l'addizione: la costante può essere il primo o il secondo operando.
      if (BO->getOpcode() == Instruction::Add) {
        if (auto *C = dyn_cast<ConstantInt>(BO->getOperand(0))) {
          auto subPair = getCanonicalForm(BO->getOperand(1));
          return {subPair.first, C->getSExtValue() + subPair.second};
        } else if (auto *C = dyn_cast<ConstantInt>(BO->getOperand(1))) {
          auto subPair = getCanonicalForm(BO->getOperand(0));
          return {subPair.first, C->getSExtValue() + subPair.second};
        }
      }
      // Per la sottrazione: consideriamo solo il caso in cui il secondo operando sia costante.
      else if (BO->getOpcode() == Instruction::Sub) {
        if (auto *C = dyn_cast<ConstantInt>(BO->getOperand(1))) {
          auto subPair = getCanonicalForm(BO->getOperand(0));
          return {subPair.first, subPair.second - C->getSExtValue()};
        }
      }
    }
    // Se V non è una add/sub "decomponibile", restituisce se stessa come base con offset 0.
    return {V, 0};
  }

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    bool Changed = false;
    // Mappa per memorizzare per ciascuna coppia (base, offset) il valore rappresentante.
    DenseMap<std::pair<Value*, int64_t>, Value*> canonicalMap;
    SmallVector<Instruction*, 8> ToRemove;

    // Scorriamo ogni BasicBlock e ogni istruzione.
    for (auto &BB : F) {
      for (auto &I : BB) {
        // Consideriamo solo operazioni binarie di addizione e sottrazione.
        if (!isa<BinaryOperator>(&I))
          continue;
        auto *BO = cast<BinaryOperator>(&I);
        if (BO->getOpcode() != Instruction::Add && BO->getOpcode() != Instruction::Sub)
          continue;

        // Calcoliamo la forma canonica.
        auto canon = getCanonicalForm(BO);

        // Se l'offset risultante è zero e la forma canonica non è l'istruzione stessa,
        // significa che l'espressione equivale semplicemente alla base.
        if (canon.second == 0 && canon.first != BO) {
          BO->replaceAllUsesWith(canon.first);
          ToRemove.push_back(BO);
          Changed = true;
          continue;
        }

        // Se esiste già un'espressione equivalente, sostituiamo questa istruzione con il rappresentante.
        auto key = canon;
        if (canonicalMap.count(key)) {
          Value *rep = canonicalMap[key];
          if (rep != BO) {
            BO->replaceAllUsesWith(rep);
            ToRemove.push_back(BO);
            Changed = true;
          }
        } else {
          canonicalMap[key] = BO;
        }
      }
    }

    // Rimuoviamo le istruzioni sostituite.
    for (Instruction *I : ToRemove) {
      I->eraseFromParent();
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
