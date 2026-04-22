# GOMOKU

## Life or Death NodeLOD (`getWinningLineColor`)

```
 X . . .    . . . .    O . . .    . . . .
 . O . .    . O . .    . X . .    . X . .
 . . O .    . . O .    . . X .    . . X .
 . . . .    . . . X    . . . .    . . . O
 
PHASE 1 (on a cell == NULL, car première fois parcouru)
step = 0
(0). . .  <-- X, O, .         def:   NODE.type = N0
 . 1 . .  <-- X, O            verif: N1 != . et N0 != N1 (next)
 . . 2 .
 . . . 3
 
PHASE 2 (on a cell != NULL et step = 1)
step = 1 LOOKING
 0 . . .
 .(1). .                      verif: N1 == N2 (next)
 . . 2 .  <-- X, O
 . . . 3
 
PHASE 3
step = 2 BOILING
 0 . . .
 . 1 . .                      verif: N2 != N3 && NODE.type != N3 (next) 
 . .(2).  <-- X, O 
 . . . 3
```

## Make group of line NodeCellRow (`getWinningLineColor`)

```
 X . . .    . . . .    O . . .    . . . .    . . . .
 . O . .    . O . .    . X . .    . . . .    . X . .
 . . O .    . . O .    . . X .    . . X .    . . X .
 . . . ?    . . . ?    . . . ?    . . . ?    . . . ?  etc...

 
PHASE 1 (on a cell == NULL ou cell == '.')
step = 0 ou 1 LOOKING
(¤). . . .  <-- .
 .(¤). . .
 . . N . .
 . . . N .
 . . . . ?
PHASE 2 (on a cell != NULL et size > 1)
step = 2 BOILING
 ¤ . . . .
 . ¤ . . .
 . .(N). .  <-- X, O
 . . .(N).
 . . . . ?
```
