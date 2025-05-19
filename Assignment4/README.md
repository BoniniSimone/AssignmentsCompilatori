
# Configurazione enviroment
Potrebbe essere che il nostro ambiente non sia confiugurato correttamente per eseguire i comandi di llvm-19.
Può essere che tali comandi siano irraggiungibili (testare con which opt). In tal caso dobbiamo aggiungere a PATH il percoroso alla cartella di llvm-19.

```bash
export LLVM_DIR="/usr/lib/llvm-19/bin"
export PATH=$LLVM_DIR:$PATH
```

# Generare IR

```bash
clang -Xclang -disable-O0-optnone -emit-llvm -S -c loopf.c -o loopfLS.ll
opt -p mem2reg loopfLS.ll -So loopfSenzaLS.ll
```

# Genera passo

```bash
clang++ -fPIC -shared -o LoopFusion.so LoopFusion.cpp `llvm-config --cxxflags --ldflags --libs core` -std=c++17
```
