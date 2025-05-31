#include <stdio.h>

void loop(int x) {
    int n=3;
    int a[3]={1, 2, 3};
    // Loop0
    for (int i = 0; i < n; i++) {
        printf("Loop0: %d\n", i);
        a[i]=a[i] + i;
    }
    

    // Loop1 (sempre eseguito)
    for (int j = 0; j < n; j++) {
        printf("Loop1: %d\n", j);

        // Aggiungo un'istruzione che dipende da loop0
        //a[j+1]=a[j] + j; //attenzione a non superare l'array bounds (modificare il numero di iterazioni in n-1)

        //Aggiungendo questa istruzione, il non si riesce a calcolare il numero di iterazioni del loop1 (SCEVCouldNotCompute)
        /*if (rand() % 2 == 0)
            break;*/
    }
}
