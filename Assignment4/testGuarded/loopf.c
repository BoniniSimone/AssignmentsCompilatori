#include <stdio.h>

void loop(int argc){
    int a[10];
    int b[10]={1,2,3,4,5,6,7,8,9,10};
    int c[10]={1,2,3,4,5,6,7,8,9,10};
    int d[10];
    int i=0;
    int j=0;

    if(argc > 0){
        do{
            a[i] = b[i] + c[i];
            i++;
        }while(i<argc);
    }
    
    if(argc > 0){
        do{
            printf("Hello World\n");
            j++;
        }while(j<argc);
    }
    
}

int main(int argc, char *argv[]) {
    loop(argc);
    printf("Loop completed successfully.\n");
    return 0;
}