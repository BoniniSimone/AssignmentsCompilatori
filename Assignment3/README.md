# Compilazione test

clang -Xclang -disable-O0-optnone -emit-llvm -S -c test.cc -o test.ll
opt -passes=mem2reg test.ll -o test.m2r.ll


# Pass + ottimizzazione

clang++ -fPIC -shared -o licmPass.so licmPass.cc `llvm-config --cxxflags --ldflags --libs core` -std=c++17

opt -load-pass-plugin ./loopCodeMotion.so -passes=licm-pass test.m2r.ll -o test.opt.ll
