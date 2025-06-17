//2. Controllare che 'I' domini tutti i suoi usi nel loop

int test_loop_invariant(int a, int b) {
    int c=0;
    int d=0;
    for (int i = 0; i < 100; i++) {
        if (a>10) 
            c = a * b; //Non domina l'uso d=c+2, perchè c'è un cammino che salta c=a*b
        
        d=c+2;
    }
    return 0;
}