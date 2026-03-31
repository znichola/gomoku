# Gomoku

Gomoku is a strategy board game traditionally played on a Go board with stones of two different colors. We will be implementing a variation that's loosly based on the [Ninuki renju rules](https://en.wikipedia.org/wiki/Gomoku#Ninuki-renju) and the [Pent rules](https://en.wikipedia.org/wiki/Gomoku#Pente).

## Reaserh notes

- [wikipedia article, with rule variations](https://en.wikipedia.org/wiki/Gomoku)
- [renju variation rules, close to doc rules](https://web.archive.org/web/20200920224708/https://renju.nu/renju-rules/)
- [paper from 1962 on a program to play gomoku](https://ebiquity.umbc.edu/paper/html/id/1130/)
- [min max](https://en.wikipedia.org/wiki/Minimax#Pseudocode)

## First meeting notes:

```
Is state on client or server?

State lives on the server, full state needs to be re-refetech after actions


What are the routes we want?

get gameState
    board grid
    stones captured black and white side
    score
    time stamp for last move, AI or human


get /startGame
    send game config
        is game human v human
    response is ok

get /move?id
    send a move by client
        is it player 1 or player two move
    response is ok

get /abandonGame
    response is ok
```