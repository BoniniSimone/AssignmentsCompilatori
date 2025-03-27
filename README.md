# Assignments

# Indice

- [Configurazione enviroment](#configurazione-enviroment)
- [Ricavare l'IR](#ricavare-lir)
- [Come compilare Asgn.cpp](#come-compilare-asgncpp)
- [Assignment 1](#assignment-1)

# Configurazione enviroment
Potrebbe essere che il nostro ambiente non sia confiugurato correttamente per eseguire i comandi di llvm-19.
Può essere che tali comandi siano irraggiungibili (testare con which opt). In tal caso dobbiamo aggiungere a PATH il percoroso alla cartella di llvm-19.

```bash
export LLVM_DIR="/usr/lib/llvm-19/bin"
export PATH = $LLVM_DIR : $PATH
```

# Ricavare l'IR

```bash
clang –O2 –emit-llvm–S –c test/Loop.c –o test/Loop.ll
```

# Come compilare Asgn.cpp
Abbiamo un file Asgn.cpp che deve diventare la nostra libreria .so che daremo come plugin a opt.
Per fare questo si utilizzi il seguente comando:


```bash
clang++ -fPIC -shared -o <nome>.so <nome>.cpp `llvm-config --cxxflags --ldflags --libs core` -std=c++17
```


# Assignment 1
Implementare tre passi LLVM che realizzano le seguenti ottimizzazioni:

> 1. AlgebraicIdentity

𝑥+ 0 = 0 + 𝑥⇒𝑥  
𝑥×1 = 1 ×𝑥⇒𝑥

> 2. StrengthReduction(più avanzato)

Effettuiamo strength reduction solo se il valore è o un multiplo di 8 o se è il suo predecessore o successore.
15 ×𝑥= 𝑥×15 ⇒(𝑥≪4) –x  
y = x / 8 ⇒y = x >> 3

> 3. Multi-InstructionOptimization

𝑎= 𝑏+ 1, 𝑐= 𝑎− 1 ⇒𝑎= 𝑏+ 1, 𝑐= 𝑏

