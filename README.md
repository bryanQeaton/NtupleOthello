# Ntuple Othello Engine with Performant Move Generator

### Todo:
- Finish Ntuple Eval
- Saving/Loading
- Tapered Ntuple Eval
- Exact Endgame solver

### Features:
- Performant Move Generation
- Simple HCE Eval Competitive with EDAX

> Design paradigm:
> Interaction with the move generator should be simple as possible (KISS)
> Eval should be trained with no lookahead


## Simple Use Case:

```c++
#include <iostream>
#include "ntuples.h"
#include "position.h"

int main() {
    generate_LUT_data(); //do at prog start
    auto pos=othello::Position();
    std::cout<<pos; //position can be output to console
    auto moves=pos.legal_moves(); //gives the legal moves 
    std::cout<<moves[0]; //moves can be done the same
    auto ps_moves=pos.psuedo_moves(pos.turn()); //gives the pseudo legal moves, this can be used for rough mobility scores
    return 0;
}

```
### Sources:
[Systematic Ntuples] Systematic Ntuple Networks for Position Evaluation

[Learning Ntuples] Learning to Play Othello with Ntuple Systems

[Coevolutionary] Learning Ntuple Networks for Othello by Coevolutionary Gradient Search



[Systematic Ntuples]: <https://arxiv.org/abs/1406.1509>
[Learning Ntuples]: <https://repository.essex.ac.uk/3820/1/NTupleOthello.pdf>
[Coevolutionary]: <https://www.cs.put.poznan.pl/mszubert/pub/krawiec2011gecco.pdf>


Made with <https://dillinger.io/>
