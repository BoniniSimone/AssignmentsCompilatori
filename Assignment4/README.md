#cerare IR

```bash
clang -Xclang -disable-O0-optnone -emit-llvm -S -c loopf.c -o loopfLS.ll
opt -p mem2reg loopfLS.ll -o loopfSenzaLS.ll
```