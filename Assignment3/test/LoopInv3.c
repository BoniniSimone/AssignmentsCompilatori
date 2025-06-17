//3. Controllare che l'istruzione domini tutti i blocchi di uscita del loop o che sia morta al di fuori del loop

int test_loop_invariant(int a, int b) {
    int c = 0;
    int d = 0;
    for (int i = 0; i < 100; i++) {
        d = c + 2; 
    }
    return d; 
}