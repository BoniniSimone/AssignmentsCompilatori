#include <stdio.h>

void loop(int x) {
    int n=1;
    // Loop0
    for (int i = 0; i < n; i++) {
        printf("Loop0: %d\n", i);
    }
    

    // Loop1 (sempre eseguito)
    for (int j = 0; j < n; j++) {
        printf("Loop1: %d\n", j);
        //Aggiungendo questa istruzione, il non si riesce a calcolare il numero di iterazioni del loop1 (SCEVCouldNotCompute)
        /*if (rand() % 2 == 0)
            break;*/
    }
}
