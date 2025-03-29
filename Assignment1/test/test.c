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

  return 0;
}