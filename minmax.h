#ifndef OTHELLO_ENGINE_H
#define OTHELLO_ENGINE_H
#include "eval.h"
#include "position.h"
#include <unordered_map>


namespace othello_engine {
    struct Entry {
        uint64_t hash=0;
        int value=-999999;
        int flag=-1;
        int depth=0;
        othello::Move best_move;
    };
    inline std::unordered_map<uint64_t,Entry> tt;
    inline othello::Move pv_table[64];
    inline int negamax(othello::Position &pos,int alpha,int beta,const int &depth) {
        if (pos.is_terminal()){return (stone_count(pos)*10000)*(!pos.turn()*2-1);} //terminal call
        //var init
        const int alph_orig=alpha;
        othello::Move best_move={false,false,0ull,0ull};
        //tt
        const uint64_t hash=pos.get_hash();
        Entry entry;
        if (tt.contains(hash)){entry=tt[hash];} //prevent creating entries when not needed
        if (entry.hash==hash) {
            if (entry.depth>=depth) {
                if (entry.flag==0){return entry.value;}
                //if (entry.flag==-1&&entry.value>=beta){return entry.value;}
                //if (entry.flag==1&&entry.value<=alpha){return entry.value;}
                if (entry.flag==-1){alpha=std::max(alpha,entry.value);}
                else if (entry.flag==1){beta=std::min(beta,entry.value);}
                if (alpha>=beta){return entry.value;}
            }
            best_move=entry.best_move;
        }
        //eval
        if (depth<=0){return eval(pos);}
        auto moves=pos.legal_moves();
        int c=0;
        bool best_move_found=false;
        bool pv_move_found=false;
        for (int m=0;m<moves.size();m++) {
            if (m!=c&&!best_move_found&&moves[m]==best_move) { //hash
                std::swap(moves[c],moves[m]);
                c++;
                best_move_found=true;
            }
            else if (m!=c&&!pv_move_found&&moves[m]==pv_table[depth]) { //pv
                std::swap(moves[c],moves[m]);
                c++;
                pv_move_found=true;
            }
            else if (m!=c&&phase(pos)>.8&&__builtin_popcountll(moves[m].flips)>__builtin_popcountll(moves[c].flips)) {
                std::swap(moves[c],moves[m]);
                c++;
            }
            else if (m!=c&&phase(pos)<.8&&__builtin_popcountll(moves[m].flips)<__builtin_popcountll(moves[c].flips)) {
                std::swap(moves[c],moves[m]);
                c++;
            }
            else if (m!=c&&psqt[__builtin_ctzll(moves[m].place)]>psqt[__builtin_ctzll(moves[c].place)]) {
                std::swap(moves[c],moves[m]);
                c++;
            }
        }
        //child nodes
        int value=-999999;
        int score;
        for (int m=0;m<moves.size();m++) {
            const othello::Move move=moves[m];
            auto new_pos=pos.make_move(move);
            if (m==0) {score=-negamax(new_pos,-beta,-alpha,depth-1);}
            else {
                score=-negamax(new_pos,-alpha-1,-alpha,depth-1); //null short
                if (score>alpha&&score<beta) {
                    score=-negamax(new_pos,-beta,-alpha,depth-1); //full
                }
            }
            if (score>value) {
                value=score;
                if (score>alpha) {
                    pv_table[depth]=moves[m];
                    best_move=moves[m];
                    if (score>=beta) {
                        break;
                    }
                    alpha=value;

                }
            }
        }
        if (depth>=entry.depth) { //depth aware replacement
            entry.value=value;
            if (value<=alph_orig){entry.flag=1;}
            else if (value>=beta){entry.flag=-1;}
            else {entry.flag=0;}
            entry.depth=depth;
            entry.hash=hash;
            entry.best_move=best_move;
            tt[hash]=entry;
        }
        return value;
    }

    inline othello::Move root(othello::Position &pos,const int &depth,const int &time_limit,bool verbose=false) {
        const auto t0=std::chrono::high_resolution_clock::now();
        auto moves=pos.legal_moves();
        if (moves.size()==1){return moves[0];}
        std::vector indices(moves.size(),0);
        std::vector values(moves.size(),0);
        std::vector depths(moves.size(),0);
        for (int d=0;d<=depth;d++) {
            for (int m=0;m<moves.size();m++) {
                indices[m]=m;
                depths[m]=d;
                auto new_pos=pos.make_move(moves[m]);
                values[m]=-negamax(new_pos,-999999,999999,d);
            }
            const auto t1=std::chrono::high_resolution_clock::now();
            if (d>1&&std::chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count()>=time_limit) {
                break;
            }
            std::vector sorted_moves(moves.size(),othello::Move());
            std::vector sorted_values(moves.size(),0);
            std::sort(indices.begin(), indices.end(),[&values](const int &i,const int &j){return values[i]>values[j];});
            for (int m=0;m<moves.size();m++) {
                sorted_moves[m]=moves[indices[m]];
                sorted_values[m]=values[indices[m]];
            }
            moves=sorted_moves;
            values=sorted_values;
        }
        if (verbose) {
            for (int m=0;m<moves.size();m++) {
                std::cout<<depths[m]<<" | "<<moves[m]<<" | "<<values[m]<<"\n";
            }
        }
        return moves[0];
    }



}




#endif //OTHELLO_ENGINE_H