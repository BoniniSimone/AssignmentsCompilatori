# Compilazione test

```bash
clang -Xclang -disable-O0-optnone -emit-llvm -S -c LoopInv.c -o LoopInv.ll
opt -p mem2reg LoopInv.ll -So LoopInv.ll
```


# Pass + ottimizzazione

```bash
clang++ -fPIC -shared -o licmPass.so licmPass.cc \`llvm-config --cxxflags --ldflags --libs core` -std=c++17
```
```bash
opt -load-pass-plugin ./licmPass.so -passes=licm-pass ./test/LoopInv.ll -o FINE.ll
```
