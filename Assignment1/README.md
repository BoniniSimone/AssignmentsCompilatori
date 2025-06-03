# Assignments

# Indice

- [Assignments](#assignments)
- [Indice](#indice)
- [Configurazione enviroment](#configurazione-enviroment)
- [Ricavare l'IR](#ricavare-lir)
- [Compilare Asgn.cpp](#compilare-asgncpp)
- [Ottimizzare:](#ottimizzare)
- [Assignment 1](#assignment-1)
    - [1. Algebraic Identity](#1-algebraic-identity)
    - [2. Strength Reduction (più avanzato)](#2-strength-reduction-più-avanzato)
    - [3. Multi-Instruction Optimization](#3-multi-instruction-optimization)

# Configurazione enviroment
Potrebbe essere che il nostro ambiente non sia confiugurato correttamente per eseguire i comandi di llvm-19.
Può essere che tali comandi siano irraggiungibili (testare con which opt). In tal caso dobbiamo aggiungere a PATH il percoroso alla cartella di llvm-19.

```bash
export LLVM_DIR="/usr/lib/llvm-19/bin"
export PATH=$LLVM_DIR:$PATH
```

# Ricavare l'IR
Questo è il metodo classico con il quale ricaviamo la rappresentazione intermedia:  
clang –O2 –emit-llvm–S –c test/Loop.c –o test/Loop.ll  
Per ricavare la rappresentazione intermedia, noi invece utiliziamo questo comando:

```bash
clang -Xclang -disable-O0-optnone -emit-llvm -S -c <nome>.c -o <nome>.ll
```
Le ottimizzazioni normali hanno molte load e store, che non ci aiutano per rimuoverle eseguire anche questo comando:
```bash
opt -p mem2reg <nomeIR>.ll -So <nomeIRsenzaLoad>.ll
```
---
*Se un file non dovesse essere leggibile perchè è in bitecode binario. Per renderlo leggibile eseguire il seguente comando:*
```bash
llvm-dis <fileNonLeggibile>.ll -o <leggibile>.ll
```

# Compilare Asgn.cpp
Abbiamo un file Asgn.cpp che deve diventare la nostra libreria .so che daremo come plugin a opt.
Per fare questo si utilizzi il seguente comando:


```bash
clang++ -fPIC -shared -o <nome>.so <nome>.cpp `llvm-config --cxxflags --ldflags --libs core` -std=c++17
```
# Ottimizzare:
Algebraic Identity:
```bash
opt -load-pass-plugin ./Asgn.so -passes=algebraic-id <nome>.ll -So <nome>.ll
```

Strength Reduction:
```bash
opt -load-pass-plugin ./Asgn.so -passes=strength-red <nome>.ll -So <nome>.ll
```

Multi-Instruction Optimization:
```bash
opt -load-pass-plugin ./Asgn.so -passes=multi-ins-opt <nome>.ll -So <nome>.ll
```

Tutti i passi insieme:
```bash
opt -load-pass-plugin ./Asgn.so -passes="algebraic-id,strength-red,multi-ins-opt" ./test/testR.ll -So ./test/testOpt.ll
```



# Assignment 1
Implementare tre passi LLVM che realizzano le seguenti ottimizzazioni:

### 1. Algebraic Identity

𝑥 + 0 = 0 + 𝑥 ⇒ 𝑥  
𝑥 × 1 = 1 × 𝑥 ⇒ 𝑥

### 2. Strength Reduction (più avanzato)

Effettuiamo strength reduction solo se il valore è o un multiplo di 8 o se è il suo predecessore o successore.  
15 &times; x = x &times; 15 ⇒ (x≪4)-x  
y = x / 8 ⇒ y = x >> 3

### 3. Multi-Instruction Optimization

𝑎=𝑏+1, 𝑐=𝑎−1 ⇒ 𝑎=𝑏+1, 𝑐=𝑏

