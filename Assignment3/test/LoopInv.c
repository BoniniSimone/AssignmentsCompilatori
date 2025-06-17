// 1. L'instruzione deve essere loop invariant (quindi i suoi operandi o sono costati o sono definiti fuori dal loop)

int test_loop_invariant(int a, int b) {
    int sum = 0;
    int c=0;
    int d=1;
    int f=0;
    for (int i = 0; i < 100; i++) {
        c = a * b;  //Operandi fuori dal loop, quindi l'istruzione è loop invariant
        f= c+2;
        d= d + i; //Non è loop invariant perchè l'operando i è definito dentro il loop
    }
    return 0;
}