#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::PatternMatch;

namespace {

  bool isPowTwo(int x) {
    return x > 0 && (x & (x - 1)) == 0;
  }

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
  // Pass 2: StrengthReduction
  // Riconosce: 15 * x e lo sostituisce con (x << 4) - x
  //-----------------------------------------------------------------------------
  struct StrengthRed : PassInfoMixin<StrengthRed> {
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
      bool Changed = false;
      for (auto &BB : F) {
        for (auto Inst = BB.begin(); Inst != BB.end(); ) {
          Instruction *I = &*Inst++;

          //Verifica se l'istruzione è una moltiplicazione
          if (auto *MulInst = dyn_cast<MulOperator>(I)) { 
            IRBuilder<> Builder(cast<Instruction>(MulInst));   // Costruisce un IRBuilder per l'istruzione corrente (IRBuilder è un builder di istruzioni IR di LLVM)

            // Otteniamo gli operandi
            Value *Op1 = MulInst->getOperand(0);          //Moltiplicando
            Value *Op2 = MulInst->getOperand(1);          //Moltiplicatore

            ConstantInt *ConstOperand = nullptr;          //Valore costante
            Value *VarOperand = nullptr;                  //Variabile

            // Controlliamo se Op1 è costante e Op2 è variabile
            if (auto *Op1Const = dyn_cast<ConstantInt>(Op1)) {
                if (!isa<ConstantInt>(Op2)) {             // Assicuriamoci che Op2 non sia costante
                  ConstOperand = Op1Const;
                  VarOperand = Op2;
                }
            }
            // Controlliamo se Op2 è costante e Op1 è variabile
            else if (auto *Op2Const = dyn_cast<ConstantInt>(Op2)) {
                if (!isa<ConstantInt>(Op1)) {             // Assicuriamoci che Op1 non sia costante
                  ConstOperand = Op2Const;
                  VarOperand = Op1;
                }
            }

            // Procediamo solo se abbiamo trovato un costante intero e una variabile
            if (ConstOperand && VarOperand) {
              int ConstValue = ConstOperand->getSExtValue();
              if (isPowTwo(ConstValue)) {
                // Sostituiamo la moltiplicazione con uno shift
                Value *Shifted = Builder.CreateShl(VarOperand, Log2_64(ConstValue)); //CreateShl(Operand1, Operand2) genera un'istruzione di shift a sinistra
                MulInst->replaceAllUsesWith(Shifted);
                cast<Instruction>(MulInst)->eraseFromParent();
                Changed = true;
              }
              else if(isPowTwo(ConstValue+1)) {
                // Sostituiamo la moltiplicazione con uno shift
                Value *Shifted = Builder.CreateShl(VarOperand, Log2_64(ConstValue+1));
                Value *Subtracted = Builder.CreateSub(Shifted, VarOperand);
                MulInst->replaceAllUsesWith(Subtracted);
                cast<Instruction>(MulInst)->eraseFromParent();
                Changed = true;
              }
              else if (isPowTwo(ConstValue-1)){
                // Sostituiamo la moltiplicazione con uno shift
                Value *Shifted = Builder.CreateShl(VarOperand, Log2_64(ConstValue-1));
                Value *Added = Builder.CreateAdd(Shifted, VarOperand);
                MulInst->replaceAllUsesWith(Added);
                cast<Instruction>(MulInst)->eraseFromParent();
                Changed = true;
              }
            }
          }

          // Verifica se l'istruzione è una divisione
          else if (auto *BinOp = dyn_cast<BinaryOperator>(I)) {
            if (BinOp->getOpcode() == Instruction::UDiv) {  // Controlla se l'operazione è una divisione Unsignes
              IRBuilder<> Builder(cast<Instruction>(BinOp));
      
              // Otteniamo gli operandi:
              Value *Dividendo = BinOp->getOperand(0);    //Dividendo
              Value *Divisore = BinOp->getOperand(1);     //Divisore
      
              // Ottimizzare solo se il divisore è costante
              if (auto *DivisorConst = dyn_cast<ConstantInt>(Divisore)) {
                if (!isa<ConstantInt>(Dividendo)) {     // Assicuriamoci che il dividendo non sia costante
                    int ConstValue = DivisorConst->getSExtValue();
                  if (isPowTwo(ConstValue)) {
                    // Sostituiamo la divisione unsigned con uno shift a destra logico
                    Value *Shifted = Builder.CreateLShr(Dividendo, Log2_64(ConstValue));
                    BinOp->replaceAllUsesWith(Shifted);
                    cast<Instruction>(BinOp)->eraseFromParent();
                    Changed = true;
                  }
                }
              }
            }
          }
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
