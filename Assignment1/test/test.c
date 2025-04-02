#include<stdio.h>

//Funzione da ottimizzare con il pass algebraic-id
void algebraic_id(){
    printf("Algebraic Identity\n");
    int m = 10;
    int n;
    n = m + 0;
    n = 0 + m;
    n = m * 1;
    n = 1 * m;

    printf("Il risultato è: %d\n", n);
}

//Funzione da ottimizzare con il pass strength-red
void strength_red(){
    unsigned int x;
    printf("Strength Reduction\n");
    printf("Inserisci un valore: ");
    scanf("%d", &x);
    
    // La moltiplicazione per 8 verrà sostituita con lo shinft di 3 posizioni a sinistra
    int m = x * 8;
    m = 8 * x;
    
    // La divisione per 8 verrà sostituita con lo shinft di 3 posizioni a destra
    unsigned int d = x / 8;

    // La moltiplicazione per 15 verrà sostituita con (x << 4) - x
    int m2 = x * 15;
    
    // La moltiplicazione per 17 verrà sostituita con (x << 4) + x
    int m3 = x * 17;

    printf("Moltiplicazione 8 * x: %d \n", m);
    printf("Divisione x / 8: %u \n", d);
    printf("Moltiplicazione x * 15: %d \n", m2);
    printf("Moltiplicazione x * 17: %d \n", m3);

}

//Funzione da ottimizzare con il pass multi-ins-opt
void multi_ins_opt(){
    int b;
    printf("Multiple Instruction Optimization\n");
    printf("Inserire un numero: ");
    scanf("%d", &b);

    int a=b-1;
    int c=a+6;
    int d=c-2;
    int e=d-3; //Questo va sostituito con e=b
    int f=d+2; //Questo va sostituito con f=c
    int g=f-1;
    int h=g+1; //Questo va sostituito con h=c

    printf("Il nuemro inserito è: %d\n", e); //Stampa e che è uguale a b 
}

int main(){
    algebraic_id();
    strength_red();
    multi_ins_opt();

    return 0;
}