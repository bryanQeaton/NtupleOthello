#include <chrono>
#include <iostream>
#include <string>
#include "eval.h"
#include "minmax.h"
#include "ntuples.h"
#include "position.h"




int main() {
    generate_LUT_data(); //do at prog start
    auto pos=othello::Position();
    auto moves=pos.legal_moves(); //gives the legal moves
    auto ps_moves=pos.psuedo_moves(pos.turn()); //gives the pseudo legal moves, this can be used for rough mobility scores
    return 0;
}
