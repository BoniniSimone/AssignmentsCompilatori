
# Esercizio 3 – Constant Propagation

## Framework

|                         | Constant Propagation                                                            |
|--------------------------|---------------------------------------------------------------------------------|
| **Domain**               | Var → Const ∪ ⊤ ∪ ⊥                                                            |
| **Direction**            | Forward                                                                         |
| **Transfer function**    | f₍ᵦ₎(x) = update(x, definizioni costanti in b)                                 |
| **Meet Operation ( ^ )** | ⋂ punto per punto: v = c₁ ∧ v = c₂ ⇒ { c se c₁ = c₂ = c ; ⊤ altrimenti }       |
| **Boundary Condition**   | IN[entry] = ∅                                                                   |
| **Initial interior points** | ∀b ≠ entry, IN[b] = ⊤                                                       |

## CFG

## Iterazioni dell’algoritmo

### Iterazione 1

| Blocco | IN       | OUT                        |
|--------|----------|----------------------------|
| BB1    | ∅        | {k=2, a=4, x=5}             |
| BB2    | ⊤        | {a=4, x=8}                  |
| BB3    | ⊤        | {b=2, x=⊤, y=⊤, k=⊤}        |

### Iterazione 2

| Blocco | IN                 | OUT                        |
|--------|--------------------|-----------------------------|
| BB1    | ∅                  | {k=2, a=4, x=5}              |
| BB2    | {k=2, a=4, x=5}    | {a=4, x=8}                   |
| BB3    | {k=2, a=4, x=5}    | {b=2, x=6, y=8, k=⊤}         |

### Iterazione 3

| Blocco | IN                 | OUT                        |
|--------|--------------------|-----------------------------|
| BB1    | ∅                  | {k=2, a=4, x=5}              |
| BB2    | {k=2, a=4, x=5}    | {a=4, x=8}                   |
| BB3    | {k=2, a=4, x=5}    | {b=2, x=6, y=8, k=⊤}         |

Poiché gli IN e OUT non cambiano più, l’algoritmo ha raggiunto un punto fisso.
