#include<stdio.h>

int main(){
    int b;
    printf("Inserire un numero: ");
    scanf("%d", &b);

    int a=b-1;
    int c=a+6;
    int d=c-2;
    int e=d-3;
    int f=d+2;
    int g=f-1;
    int h=g+1;
    

    printf("Il nuemro inserito è: %d\n", e);
    return 0;
}