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
    }

    for (int k = 0; k < n; k++) {
        printf("Loop2: %d\n", k);
    }
}
