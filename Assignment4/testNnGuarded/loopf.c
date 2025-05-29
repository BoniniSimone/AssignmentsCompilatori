#include <stdio.h>

void loop(int argc){
    int a[10];
    int b[10]={1,2,3,4,5,6,7,8,9,10};
    int c[10]={1,2,3,4,5,6,7,8,9,10};
    int d[10];
    int i=0;
    int j=0;

    for(i = 0; i < 10; i++) {
        a[i] = b[i] + c[i];
    }
    
    for(j = 0; j < 10; j++) {
        a[j] = b[j] + c[j];
    }
    
}