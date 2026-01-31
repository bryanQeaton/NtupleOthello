#ifndef OTHELLO_EVAL_H
#define OTHELLO_EVAL_H
#include "position.h"

namespace othello_engine {
    inline float count_coef=1;
    inline float mobility_coef=3.5;
    inline float psqt_coef=.05;
    inline int psqt[64]={
        640,-380, 180, 80,  80,  180,-380, 640,
       -380,-520,-100,-100,-100,-100,-520,-380,
        180,-100, 20,  20,  20,  20, -100, 180,
        80, -100, 20,  0,   0,   20, -100, 80,
        80, -100, 20,  0,   0,   20, -100, 80,
        180,-100, 20,  20,  20,  20,- 100, 180,
       -380,-520,-100,-100,-100,-100,-520,-380,
        640,-380, 180, 80,  80,  180,-380, 640
   };
    inline float phase(const othello::Position &pos){return __builtin_popcountll(pos.occ())/64.f;}
    inline int stone_count(const othello::Position &pos) {
        return __builtin_popcountll(pos.side_occ(false))-__builtin_popcountll(pos.side_occ(true));
    }
    inline int mobility(const othello::Position &pos) {
        return __builtin_popcountll(pos.psuedo_moves(false))-__builtin_popcountll(pos.psuedo_moves(true));
    }
    inline int psqt_score(const othello::Position &pos) {
        uint64_t black=pos.side_occ(false);
        uint64_t white=pos.side_occ(true);
        int score=0;
        while (black) {
            const int sqr=__builtin_ctzll(black);
            score+=psqt[sqr];
            black&=black-1;
        }
        while (white) {
            const int sqr=__builtin_ctzll(white);
            score-=psqt[sqr];
            white&=white-1;
        }
        return score;
    }

    inline int eval(const othello::Position &pos) {
        return (
            stone_count(pos)*phase(pos)*count_coef
            +mobility(pos)*mobility_coef
            +psqt_score(pos)*(1-phase(pos))*psqt_coef
            )*(!pos.turn()*2-1);
    }
}

#endif //OTHELLO_EVAL_H