.PHONY: all clean

# Target principale: genera l'eseguibile
all: ./test/eseguibile

# 1. Compila il file C in LLVM IR (.ll)
./test/test.ll: ./test/test.c
	clang -Xclang -disable-O0-optnone -emit-llvm -S -c ./test/test.c -o ./test/test.ll

# 2. Esegui mem2reg sul file .ll per ottenere il file pulito
./test/testCln.ll: ./test/test.ll
	opt -passes=mem2reg ./test/test.ll -o ./test/testCln.ll

# 3. Disassemble del file LLVM ottimizzato
./test/testR.ll: ./test/testCln.ll
	llvm-dis ./test/testCln.ll -o ./test/testR.ll

# 4. Compila il file C++ in una shared object (plugin LLVM)
Asgn.so: Asgn.cpp
	clang++ -fPIC -shared -o Asgn.so Asgn.cpp `llvm-config --cxxflags --ldflags --libs core` -std=c++17

# 5. Esegui il pass plugin sul file .ll
./test/testOpt.ll: ./test/testR.ll Asgn.so
	opt -load-pass-plugin ./Asgn.so -passes=multi-ins-opt ./test/testR.ll -o ./test/testOpt.ll

# 6. Disassemble del file ottimizzato dopo il pass plugin
./test/testOptR.ll: ./test/testOpt.ll
	llvm-dis ./test/testOpt.ll -o ./test/testOptR.ll

# 7. Compila il file LLVM IR finale in un eseguibile
./test/eseguibile: ./test/testOptR.ll
	clang -O0 ./test/testOptR.ll -o ./test/eseguibile

# Target per la pulizia: elimina l'eseguibile e tutti i file .ll in ./test
clean:
	rm -f ./test/eseguibile ./test/*.ll