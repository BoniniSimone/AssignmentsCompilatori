#include <stdio.h>

void loop(){
    int a[10];
    int b[10]={1,2,3,4,5,6,7,8,9,10};
    int c[10]={1,2,3,4,5,6,7,8,9,10};
    int d[10];
    
    for (int i=0; i<10; i++){
        a[i] = b[i] + c[i];
    }

    for (int i=0; i<10; i++){
        d[i] = a[i] + 1;
    }
}