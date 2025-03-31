#include <stdio.h>

int main() {
  unsigned int x;
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

  return 0;
}