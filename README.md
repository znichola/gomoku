# Gomoku

Gomoku is a strategy board game traditionally played on a Go board with stones of two different colors.

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