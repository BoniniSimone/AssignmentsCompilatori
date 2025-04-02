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

  bool isPowTwo(int x) {
    return x > 0 && (x & (x - 1)) == 0;
  }

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
struct MultiInsOpt : public PassInfoMixin<MultiInsOpt> {

  /// Funzione per ottenere la forma canonica (base, offset)
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

    //canonicalMap memorizza ciascuna coppia (base, offset) come chiave e l'istruzione rappresentante. es: a=b+15 {(b, 15) -> a}
    DenseMap<std::pair<Value*, int64_t>, Value*> canonicalMap; 
    SmallVector<Instruction*, 8> ToRemove; //memoriziamo le istruzioni da rimuovere.

    // Scorriamo ogni BasicBlock e ogni istruzione.
    for (auto &BB : F) {
      for (auto &I : BB) {
        // Consideriamo solo operazioni binarie di addizione e sottrazione.
        if (!isa<BinaryOperator>(&I)) continue;
        auto *BO = cast<BinaryOperator>(&I);
        if (BO->getOpcode() != Instruction::Add && BO->getOpcode() != Instruction::Sub) continue;

        // Calcoliamo la forma canonica.
        auto canon = getCanonicalForm(BO);

        // Se l'offset è zero e la base non è l'istruzione stessa, l'istruzione si può sostituire con la base
        if (canon.second == 0 && canon.first != BO) {
          BO->replaceAllUsesWith(canon.first);
          ToRemove.push_back(BO);
          Changed = true;
          continue;
        }

        // Se esiste già un istruzione con la stessa forma (base, valore), sostituiamo l'istruzione BO con la sua equivalente presente nella canonicalMap.
        if (canonicalMap.count(canon)) {    //ritorna sempre 0 o 1
          Value *rep = canonicalMap[canon];
          if (rep != BO) {
            BO->replaceAllUsesWith(rep);
            ToRemove.push_back(BO);
            Changed = true;
          }
          //Se l'istruzione non è presente nella canonicalMap, la aggiungiamo
        } else {
          canonicalMap[canon] = BO;
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
          [](StringRef Names, FunctionPassManager &FPM,
             ArrayRef<PassBuilder::PipelineElement>) {
            // Separa i nomi dei passaggi in base alla virgola
            SmallVector<StringRef, 4> PassNames;
            Names.split(PassNames, ',');
            bool handled = false;
            for (auto Name : PassNames) {
              Name = Name.trim(); // Rimuove eventuali spazi
              if (Name == "algebraic-id") {
                FPM.addPass(AlgebraicId());
                handled = true;
              } else if (Name == "strength-red") {
                FPM.addPass(StrengthRed());
                handled = true;
              } else if (Name == "multi-ins-opt") {
                FPM.addPass(MultiInsOpt());
                handled = true;
              }
            }
            return handled;
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
