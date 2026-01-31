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

    train(100,.75f);
    train(200,.50f);
    train(300,.25f);
    train(400,.125f);
    train(500,.0612f);
    train(600,.0306f);
    train(700,.0015f);
    train(800,.0008f);
    train(900,.0004f);
    while (!pos.is_terminal()) {
        std::cout<<pos;
        auto m=pos.legal_moves();
        if (m.size()==1) {
            pos=pos.make_move(m[0]);
        }
        else {
            std::string player_move;
            std::cin>>player_move;
            pos=pos.make_move(player_move);
        }
        std::cout<<pos;
        auto moves=pos.legal_moves();
        othello::Move best_move;
        float best_value=-1e9;
        for (auto move:moves) {
            auto next=pos.make_move(move);
            float val=n_eval(next);
            if (val>best_value) {
                best_value=val;
                best_move=move;
            }
        }
        pos=pos.make_move(best_move);
    }
    std::cout<<othello_engine::stone_count(pos)<<" :X - O\n";


    return 0;
}
