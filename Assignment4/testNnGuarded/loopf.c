#include <stdio.h>

void loop(int x) {
    
    // Loop0
    for (int i = 0; i < 10; i++) {
        printf("Loop0: %d\n", i);
    }
    

    // Loop1 (sempre eseguito)
    for (int j = 0; j < 10; j++) {
        printf("Loop1: %d\n", j);
    }
}
