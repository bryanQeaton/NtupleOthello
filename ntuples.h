#ifndef OTHELLO_NTUPLES_H
#define OTHELLO_NTUPLES_H
#include <cstdint>
#include <vector>
#include <intrin.h>
#include <algorithm>
#include <complex>

#include "position.h"


inline std::uniform_real_distribution<float> real_dist(0,1);
inline uint64_t flip_vert(const uint64_t &x) {
    uint64_t r=0ull;
    r=((x>>8)&0x00FF00FF00FF00FF)|((x&0x00FF00FF00FF00FF)<<8);
    r=((r>>16)&0x0000FFFF0000FFFF)|((r&0x0000FFFF0000FFFF)<<16);
    return (r>>32)|(r<<32);
}
inline uint64_t flip_horiz(const uint64_t &x) {
    constexpr uint64_t k1=0x5555555555555555;
    constexpr uint64_t k2=0x3333333333333333;
    constexpr uint64_t k4=0x0f0f0f0f0f0f0f0f;
    uint64_t r=((x>>1)&k1)|((x&k1)<<1);
    r=((r>>2)&k2)|((r&k2)<<2);
    return ((r>>4)&k4)|((r&k4)<<4);
}
inline uint64_t flip_diag(const uint64_t &x) {
    uint64_t t=0ull;
    uint64_t r=x;
    constexpr uint64_t k1=0x5500550055005500;
    constexpr uint64_t k2=0x3333000033330000;
    constexpr uint64_t k4=0x0f0f0f0f00000000;
    t=(x^(x<<7))&k1;r^=t^(t>>7);
    t=(r^(r<<14))&k2;r^=t^(t>>14);
    t=(r^(r<<28))&k4;r^=t^(t>>28);
    return r;
}
inline std::vector<uint64_t> symmetries(const uint64_t &x) {
    std::vector<uint64_t> r;
    r.push_back(x);
    r.push_back(flip_vert(x));
    r.push_back(flip_diag(x));
    r.push_back(flip_horiz(x));
    r.push_back(flip_vert(r[2]));
    r.push_back(flip_vert(r[3]));
    r.push_back(flip_horiz(r[2]));
    r.push_back(flip_vert(r[6]));
    return r;
}
inline int index_table[256][256];
inline int index(const uint64_t &stm,const uint64_t &opp, const uint64_t &mask) { //gets the index in a lookup table for a given mask
    return index_table[_pext_u64(stm,mask)][_pext_u64(opp,mask)];
}
inline uint64_t LUT1_MASKS[10];
inline uint64_t LUT2_MASKS[32];
inline uint64_t LUT3_MASKS[24];
inline uint64_t LUT4_MASKS[31];
inline uint64_t LUT5_MASKS[14];
inline uint64_t LUT6_MASKS[24];
inline uint64_t LUT7_MASKS[6];
inline uint64_t LUT8_MASKS[17];
inline float LUT1[10][3]={}; //3^1 //cut these down to uint8 and quantize for speed?
inline float LUT2[32][9]={}; //3^2
inline float LUT3[24][27]={}; //3^3
inline float LUT4[31][81]={}; //3^4
inline float LUT5[14][243]={}; //3^5
inline float LUT6[24][729]={}; //3^6
inline float LUT7[6][2187]={}; //3^7
inline float LUT8[17][6561]={}; //3^8
inline void generate_LUT_data() { //generates LUT masks
    constexpr uint64_t edge_masks[4]={0xff00000000000000,0x101010101010101,0xff,0x8080808080808080}; //north east south west
    std::vector<std::vector<uint64_t>> nmasks(8);
    for (int i=0;i<64;i++) {
        nmasks[0].push_back(1ull<<i);
        uint64_t sqr=1ull<<i;
        uint64_t mask[8]={sqr,sqr,sqr,sqr,sqr,sqr,sqr,sqr};
        for (int n=1;n<=8;n++) { //this will create duplicates
            mask[0]|=(mask[0]<<1)&~edge_masks[1]; //east facing
            if (mask[0])nmasks[__builtin_popcountll(mask[0])-1].push_back(mask[0]);
            mask[1]|=(mask[1]>>1)&~edge_masks[3]; //west facing
            if (mask[1])nmasks[__builtin_popcountll(mask[1])-1].push_back(mask[1]);
            mask[2]|=(mask[2]<<1*8)&~edge_masks[2]; //south facing
            if (mask[2])nmasks[__builtin_popcountll(mask[2])-1].push_back(mask[2]);
            mask[3]|=(mask[3]>>1*8)&~edge_masks[0]; //north facing
            if (mask[3])nmasks[__builtin_popcountll(mask[3])-1].push_back(mask[3]);
            mask[4]|=(mask[4]&~(edge_masks[3]|edge_masks[0]))<<1*9; //nw facing
            if (mask[4])nmasks[__builtin_popcountll(mask[4])-1].push_back(mask[4]);
            mask[5]|=(mask[5]&~(edge_masks[1]|edge_masks[0]))<<1*7; //ne facing
            if (mask[5])nmasks[__builtin_popcountll(mask[5])-1].push_back(mask[5]);
            mask[6]|=(mask[6]&~(edge_masks[1]|edge_masks[2]))>>1*9; //sw facing
            if (mask[6])nmasks[__builtin_popcountll(mask[6])-1].push_back(mask[6]);
            mask[7]|=(mask[7]&~(edge_masks[3]|edge_masks[2]))>>1*7; //se facing
            if (mask[7])nmasks[__builtin_popcountll(mask[7])-1].push_back(mask[7]);
        }
    }
    //blocks: 2x2 2x3 2x4
    uint64_t pattern=0x303;
    for (int i=0;i<64-8;i++) {
        if (i%8==7){continue;}
        uint64_t p=pattern<<i;
        nmasks[3].push_back(p);
    }
    pattern=0x707;
    for (int i=0;i<64-8;i++) {
        if (i%8==7){continue;}
        if (i%8==6){continue;}
        uint64_t p=pattern<<i;
        nmasks[5].push_back(p);
    }
    pattern=0x30303;
    for (int i=0;i<64-16;i++) {
        if (i%8==7){continue;}
        uint64_t p=pattern<<i;
        nmasks[5].push_back(p);
    }
    pattern=0xf0f;
    for (int i=0;i<64-8;i++) {
        if (i%8==7){continue;}
        if (i%8==6){continue;}
        if (i%8==5){continue;}
        uint64_t p=pattern<<i;
        nmasks[7].push_back(p);
    }
    pattern=0x3030303;
    for (int i=0;i<64-24;i++) {
        if (i%8==7){continue;}
        uint64_t p=pattern<<i;
        nmasks[7].push_back(p);
    }
    //remove duplicates
    for (int i=0;i<8;i++) {
        std::sort(nmasks[i].begin(),nmasks[i].end());
        nmasks[i].erase(std::unique(nmasks[i].begin(),nmasks[i].end()),nmasks[i].end());
    }
    std::vector<std::vector<uint64_t>> nmasks_sym(8);
    for (int i=0;i<8;i++) {
        for (int n=0;n<nmasks[i].size();n++) {
            auto sym=symmetries(nmasks[i][n]);
            //get smallest
            uint64_t smallest=~0ull;
            for (auto m:sym) {
                if (m<smallest){smallest=m;}
            }
            nmasks_sym[i].push_back(smallest);
        }
    }
    for (int i=0;i<8;i++) {
        std::sort(nmasks_sym[i].begin(),nmasks_sym[i].end());
        nmasks_sym[i].erase(std::unique(nmasks_sym[i].begin(),nmasks_sym[i].end()),nmasks_sym[i].end());
    }
    for (int n=0;n<256;n++) {
        for (int m=0;m<256;m++) {
            const int index=
                (m&1)*1+(n&1)*2+
                (m>>1&1)*3+(n>>1&1)*6+
                (m>>2&1)*9+(n>>2&1)*18+
                (m>>3&1)*27+(n>>3&1)*54+
                (m>>4&1)*81+(n>>4&1)*162+
                (m>>5&1)*243+(n>>5&1)*486+
                (m>>6&1)*729+(n>>6&1)*1458+
                (m>>7&1)*2187+(n>>7&1)*4374;
            index_table[n][m]=index;
        }
    }
}
inline void symmetries_fast(const uint64_t &x, uint64_t out[8]){
    out[0]=x;
    out[1]=flip_vert(x);
    out[2]=flip_diag(x);
    out[3]=flip_horiz(x);
    out[4]=flip_vert(out[2]);
    out[5]=flip_vert(out[3]);
    out[6]=flip_horiz(out[2]);
    out[7]=flip_vert(out[6]);
}
inline float n_eval(const othello::Position &pos) {
    float r=0.f;
    uint64_t stm_sym[8], opp_sym[8];
    symmetries_fast(pos.side_occ(pos.turn()), stm_sym);
    symmetries_fast(pos.side_occ(!pos.turn()), opp_sym);
    for (int i=0;i<8;i++) {
        const uint64_t stm=stm_sym[i];
        const uint64_t opp=opp_sym[i];
        r+=LUT1[0][index(stm,opp,LUT1_MASKS[0])];
        r+=LUT1[1][index(stm,opp,LUT1_MASKS[1])];
        r+=LUT1[2][index(stm,opp,LUT1_MASKS[2])];
        r+=LUT1[3][index(stm,opp,LUT1_MASKS[3])];
        r+=LUT1[4][index(stm,opp,LUT1_MASKS[4])];
        r+=LUT1[5][index(stm,opp,LUT1_MASKS[5])];
        r+=LUT1[6][index(stm,opp,LUT1_MASKS[6])];
        r+=LUT1[7][index(stm,opp,LUT1_MASKS[7])];
        r+=LUT1[8][index(stm,opp,LUT1_MASKS[8])];
        r+=LUT1[9][index(stm,opp,LUT1_MASKS[9])];
        for (int n=0;n<32;n++) {r+=LUT2[n][index(stm,opp,LUT2_MASKS[n])];}
        for (int n=0;n<24;n++) {r+=LUT3[n][index(stm,opp,LUT3_MASKS[n])];}
        for (int n=0;n<31;n++) {r+=LUT4[n][index(stm,opp,LUT4_MASKS[n])];}
        for (int n=0;n<14;n++) {r+=LUT5[n][index(stm,opp,LUT5_MASKS[n])];}
        for (int n=0;n<24;n++) {r+=LUT6[n][index(stm,opp,LUT6_MASKS[n])];}
        r+=LUT7[0][index(stm,opp,LUT7_MASKS[0])];
        r+=LUT7[1][index(stm,opp,LUT7_MASKS[1])];
        r+=LUT7[2][index(stm,opp,LUT7_MASKS[2])];
        r+=LUT7[3][index(stm,opp,LUT7_MASKS[3])];
        r+=LUT7[4][index(stm,opp,LUT7_MASKS[4])];
        r+=LUT7[5][index(stm,opp,LUT7_MASKS[5])];
        for (int n=0;n<17;n++) {r+=LUT8[n][index(stm,opp,LUT8_MASKS[n])];}
    }
    return r;
}
//train function
//play game through to the end using eval and random chance
//getting the affected LUTs is done using the index function

//like its the opposite of the eval, just index the LUTS and
//add to them instead of taking
inline void train(const int &games,const int &random_move_chance) {
     for (int i=0;i<games;i++) {
        auto pos=othello::Position();
        //gameloop
        while (!pos.is_terminal()) {
            othello::Move best_move;
            auto moves=pos.legal_moves();
            //find best more or random chance move
            if (real_dist(gen)>=random_move_chance) {
                float best_value=-1e9;
                for (auto move:moves) {
                    auto n=pos.make_move(move);
                    float val=n_eval(n);
                    if (val>best_value) {
                        best_value=val;
                        best_move=move;
                    }
                }
            }
            else {
                best_move=moves[int_dist(gen)%moves.size()];
            }
            auto next=pos.make_move(best_move);
            const float cur=n_eval(pos);
            float target;
            if (next.is_terminal()) {
                const int c=othello_engine::stone_count(pos);
                if (c>0){target=-1.f;}
                else if (c<0){target=1.f;}
                else if (c==0){target=0.f;}
            }
            else {
                target=-.99f*n_eval(pos);
            }
            float err=target-cur;
            uint64_t stm_sym[8], opp_sym[8];
            symmetries_fast(pos.side_occ(pos.turn()), stm_sym);
            symmetries_fast(pos.side_occ(!pos.turn()), opp_sym);
            for (int i=0;i<8;i++) {
                const uint64_t stm=stm_sym[i];
                const uint64_t opp=opp_sym[i];
                LUT1[0][index(stm,opp,LUT1_MASKS[0])]+=0.01f*err;
                LUT1[1][index(stm,opp,LUT1_MASKS[1])]+=0.01f*err;
                LUT1[2][index(stm,opp,LUT1_MASKS[2])]+=0.01f*err;
                LUT1[3][index(stm,opp,LUT1_MASKS[3])]+=0.01f*err;
                LUT1[4][index(stm,opp,LUT1_MASKS[4])]+=0.01f*err;
                LUT1[5][index(stm,opp,LUT1_MASKS[5])]+=0.01f*err;
                LUT1[6][index(stm,opp,LUT1_MASKS[6])]+=0.01f*err;
                LUT1[7][index(stm,opp,LUT1_MASKS[7])]+=0.01f*err;
                LUT1[8][index(stm,opp,LUT1_MASKS[8])]+=0.01f*err;
                LUT1[9][index(stm,opp,LUT1_MASKS[9])]+=0.01f*err;
                for (int n=0;n<32;n++) {LUT2[n][index(stm,opp,LUT2_MASKS[n])]+=0.01f*err;}
                for (int n=0;n<24;n++) {LUT3[n][index(stm,opp,LUT3_MASKS[n])]+=0.01f*err;}
                for (int n=0;n<31;n++) {LUT4[n][index(stm,opp,LUT4_MASKS[n])]+=0.01f*err;}
                for (int n=0;n<14;n++) {LUT5[n][index(stm,opp,LUT5_MASKS[n])]+=0.01f*err;}
                for (int n=0;n<24;n++) {LUT6[n][index(stm,opp,LUT6_MASKS[n])]+=0.01f*err;}
                LUT7[0][index(stm,opp,LUT7_MASKS[0])]+=0.01f*err;
                LUT7[1][index(stm,opp,LUT7_MASKS[1])]+=0.01f*err;
                LUT7[2][index(stm,opp,LUT7_MASKS[2])]+=0.01f*err;
                LUT7[3][index(stm,opp,LUT7_MASKS[3])]+=0.01f*err;
                LUT7[4][index(stm,opp,LUT7_MASKS[4])]+=0.01f*err;
                LUT7[5][index(stm,opp,LUT7_MASKS[5])]+=0.01f*err;
                for (int n=0;n<17;n++) {LUT8[n][index(stm,opp,LUT8_MASKS[n])]+=0.01f*err;}
            }
            pos=next;
        }
    }
}


#endif //OTHELLO_NTUPLES_H